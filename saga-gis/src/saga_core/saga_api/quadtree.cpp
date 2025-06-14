
///////////////////////////////////////////////////////////
//                                                       //
//                         SAGA                          //
//                                                       //
//      System for Automated Geoscientific Analyses      //
//                                                       //
//           Application Programming Interface           //
//                                                       //
//                  Library: SAGA_API                    //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//                     quadtree.cpp                      //
//                                                       //
//          Copyright (C) 2009 by Olaf Conrad            //
//                                                       //
//-------------------------------------------------------//
//                                                       //
// This file is part of 'SAGA - System for Automated     //
// Geoscientific Analyses'.                              //
//                                                       //
// This library is free software; you can redistribute   //
// it and/or modify it under the terms of the GNU Lesser //
// General Public License as published by the Free       //
// Software Foundation, either version 2.1 of the        //
// License, or (at your option) any later version.       //
//                                                       //
// This library is distributed in the hope that it will  //
// be useful, but WITHOUT ANY WARRANTY; without even the //
// implied warranty of MERCHANTABILITY or FITNESS FOR A  //
// PARTICULAR PURPOSE. See the GNU Lesser General Public //
// License for more details.                             //
//                                                       //
// You should have received a copy of the GNU Lesser     //
// General Public License along with this program; if    //
// not, see <http://www.gnu.org/licenses/>.              //
//                                                       //
//-------------------------------------------------------//
//                                                       //
//    e-mail:     oconrad@saga-gis.org                   //
//                                                       //
//    contact:    Olaf Conrad                            //
//                Institute of Geography                 //
//                University of Hamburg                  //
//                Germany                                //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
#include "shapes.h"


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_PRQuadTree_Node::CSG_PRQuadTree_Node(const CSG_Rect &Extent, int Quadrant)
	: CSG_PRQuadTree_Item(Extent, Quadrant)
{
	m_pChildren[0] = m_pChildren[1] = m_pChildren[2] = m_pChildren[3] = NULL;
}

//---------------------------------------------------------
CSG_PRQuadTree_Node::CSG_PRQuadTree_Node(CSG_PRQuadTree_Leaf *pLeaf)
	: CSG_PRQuadTree_Item(pLeaf->m_Extent)
{
	m_pChildren[0] = m_pChildren[1] = m_pChildren[2] = m_pChildren[3] = NULL;

	int	Quadrant	=  Get_Quadrant(pLeaf->Get_Point());

	pLeaf->Set_Extent(m_Extent, Quadrant);

	m_pChildren[Quadrant] = pLeaf;
}

//---------------------------------------------------------
CSG_PRQuadTree_Node::~CSG_PRQuadTree_Node(void)
{
	for(int i=0; i<4; i++)
	{
		if( m_pChildren[i] )
		{
			if( m_pChildren[i]->is_Leaf() )
			{
				delete((CSG_PRQuadTree_Leaf *)m_pChildren[i]);
			}
			else
			{
				delete((CSG_PRQuadTree_Node *)m_pChildren[i]);
			}
		}
	}
}

//---------------------------------------------------------
CSG_PRQuadTree_Item * CSG_PRQuadTree_Node::Get_Child(double x, double y)
{
	for(int i=0; i<4; i++)
	{
		if( m_pChildren[i] && m_pChildren[i]->Contains(x, y) )
		{
			return( m_pChildren[i]->is_Node() ? m_pChildren[i]->asNode()->Get_Child(x, y) : m_pChildren[i] );
		}
	}

	return( this );
}

//---------------------------------------------------------
bool CSG_PRQuadTree_Node::Add_Point(double x, double y, double z)
{
	if( !Contains(x, y) )
	{
		return( false );
	}

	if( has_Statistics() )
	{
		Get_X()->Add_Value(x);
		Get_Y()->Add_Value(y);
		Get_Z()->Add_Value(z);
	}

	int	Quadrant	= Get_Quadrant(x, y);

	//-----------------------------------------------------
	if( m_pChildren[Quadrant] == NULL )
	{
		m_pChildren[Quadrant]	= new CSG_PRQuadTree_Leaf(m_Extent, Quadrant, x, y, z);

		return( true );
	}

	//-----------------------------------------------------
	if( m_pChildren[Quadrant]->is_Leaf() )
	{
		CSG_PRQuadTree_Leaf	*pLeaf	= m_pChildren[Quadrant]->asLeaf();

		if( x != pLeaf->Get_X() || y != pLeaf->Get_Y() )
		{
			if( has_Statistics() )
			{
				m_pChildren[Quadrant]	= new CSG_PRQuadTree_Node_Statistics(pLeaf);
			}
			else
			{
				m_pChildren[Quadrant]	= new CSG_PRQuadTree_Node(pLeaf);
			}

			((CSG_PRQuadTree_Node *)m_pChildren[Quadrant])->Add_Point(x, y, z);
		}
		else // if( x == pLeaf->Get_X() || y == pLeaf->Get_Y() ) // duplicate !!
		{
			if( !pLeaf->has_Statistics() )
			{
				m_pChildren[Quadrant]	= new CSG_PRQuadTree_Leaf_List(pLeaf->m_Extent, -1, x, y, pLeaf->m_z);

				delete(pLeaf);
			}

			((CSG_PRQuadTree_Leaf_List *)m_pChildren[Quadrant])->Add_Value(z);
		}

		return( true );
	}

	//-----------------------------------------------------
	return( ((CSG_PRQuadTree_Node *)m_pChildren[Quadrant])->Add_Point(x, y, z) );	//	if( m_pChildren[i]->is_Node() )
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_PRQuadTree::CSG_PRQuadTree(void)
{
	m_pRoot		= NULL;
	m_nPoints	= 0;
	m_bPolar	= false;
}

//---------------------------------------------------------
CSG_PRQuadTree::CSG_PRQuadTree(const TSG_Rect &Extent, bool bStatistics)
{
	m_pRoot		= NULL;
	m_nPoints	= 0;
	m_bPolar	= false;

	Create(Extent, bStatistics);
}

//---------------------------------------------------------
CSG_PRQuadTree::CSG_PRQuadTree(CSG_Shapes *pShapes, int Attribute, bool bStatistics)
{
	m_pRoot		= NULL;
	m_nPoints	= 0;
	m_bPolar	= false;

	Create(pShapes, Attribute, bStatistics);
}

//---------------------------------------------------------
CSG_PRQuadTree::~CSG_PRQuadTree(void)
{
	Destroy();
}

//---------------------------------------------------------
bool CSG_PRQuadTree::Create(const CSG_Rect &Extent, bool bStatistics)
{
	Destroy();

	if( Extent.Get_XRange() > 0.0 && Extent.Get_YRange() > 0.0 )
	{
		double	Size	= (0.5 + 0.01) * (Extent.Get_XRange() > Extent.Get_YRange() ? Extent.Get_XRange() : Extent.Get_YRange());

		CSG_Rect	r(
			Extent.Get_XCenter() - Size, Extent.Get_YCenter() - Size,
			Extent.Get_XCenter() + Size, Extent.Get_YCenter() + Size
		);

		if( bStatistics )
		{
			m_pRoot	= new CSG_PRQuadTree_Node_Statistics(r);
		}
		else
		{
			m_pRoot	= new CSG_PRQuadTree_Node(r);
		}

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_PRQuadTree::Create(CSG_Shapes *pShapes, int Attribute, bool bStatistics)
{
	Destroy();

	if( pShapes && pShapes->is_Valid() && Create(pShapes->Get_Extent(), bStatistics) )
	{
		for(sLong iShape=0; iShape<pShapes->Get_Count() && SG_UI_Process_Set_Progress(iShape, pShapes->Get_Count()); iShape++)
		{
			CSG_Shape *pShape = pShapes->Get_Shape(iShape);

			if( Attribute < 0 || !pShape->is_NoData(Attribute) )
			{
				double z = Attribute < 0 ? iShape : pShape->asDouble(Attribute);

				for(int iPart=0; iPart<pShape->Get_Part_Count(); iPart++)
				{
					for(int iPoint=0; iPoint<pShape->Get_Point_Count(iPart); iPoint++)
					{
						Add_Point(pShape->Get_Point(iPoint, iPart), z);
					}
				}
			}
		}

		return( m_nPoints > 0 );
	}

	return( false );
}

//---------------------------------------------------------
void CSG_PRQuadTree::Destroy(void)
{
	if( m_pRoot )
	{
		delete(m_pRoot);

		m_pRoot	= NULL;
	}

	m_nPoints	= 0;

	m_Selection.Destroy();
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PRQuadTree::Add_Point(double x, double y, double z)
{
	if( _Check_Root(x, y) && m_pRoot->Add_Point(x, y, z) )
	{
		m_nPoints++;

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
bool CSG_PRQuadTree::Add_Point(const TSG_Point &p, double z)
{
	return( Add_Point(p.x, p.y, z) );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
bool CSG_PRQuadTree::_Check_Root(double x, double y)
{
	if( !m_pRoot )
	{
		return( false );
	}
	
	if( m_pRoot->Get_Extent().Contains(x, y) )
	{
		return( true );
	}

	//-----------------------------------------------------
	int	Quadrant	= y < m_pRoot->Get_yMin()	// bottom / top ?
		? (x < m_pRoot->Get_xMin() ? 0 : 3) 	// bottom: left / right ?
		: (x < m_pRoot->Get_xMin() ? 1 : 2);	// top   : left / right ?
		
	TSG_Rect	Extent	= m_pRoot->Get_Extent();

	switch( Quadrant )
	{
	case 0: Extent.xMin -= m_pRoot->Get_Size(); Extent.yMin -= m_pRoot->Get_Size(); break; // bottom left
	case 1: Extent.xMin -= m_pRoot->Get_Size(); Extent.yMax += m_pRoot->Get_Size(); break; // top left
	case 2: Extent.xMax += m_pRoot->Get_Size(); Extent.yMin -= m_pRoot->Get_Size(); break; // bottom right
	case 3: Extent.xMax += m_pRoot->Get_Size(); Extent.yMax += m_pRoot->Get_Size(); break; // top right
	}

	CSG_PRQuadTree_Node	*pRoot;

	if( m_pRoot->has_Statistics() )
	{
		CSG_PRQuadTree_Node_Statistics	*pNode	= new CSG_PRQuadTree_Node_Statistics(Extent);

		pNode->m_x	= ((CSG_PRQuadTree_Node_Statistics *)m_pRoot)->m_x;
		pNode->m_y	= ((CSG_PRQuadTree_Node_Statistics *)m_pRoot)->m_y;
		pNode->m_z	= ((CSG_PRQuadTree_Node_Statistics *)m_pRoot)->m_z;

		pRoot	= pNode;
	}
	else
	{
		pRoot	= new CSG_PRQuadTree_Node(Extent);
	}

	pRoot->m_pChildren[Quadrant]	= m_pRoot;

	m_pRoot	= pRoot;

	return( _Check_Root(x, y) );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
inline bool CSG_PRQuadTree::_Quadrant_Contains(double x, double y, int iQuadrant, const TSG_Point &p) const
{
	switch( iQuadrant )
	{
	case 0:	return( x <  p.x && y <  p.y );	// lower left
	case 1:	return( x <  p.x && y >= p.y );	// upper left
	case 2:	return( x >= p.x && y >= p.y );	// upper right
	case 3:	return( x >= p.x && y <  p.y );	// lower right
	}

	return( true );
}

//---------------------------------------------------------
inline bool CSG_PRQuadTree::_Radius_Contains(double x, double y, double r, const TSG_Point &p)	const
{
	double	dx, dy;

	if( fabs(dx = x - p.x) <= r && fabs(dy = y - p.y) <= r )
	{
		return( dx*dx + dy*dy < r*r );
	}

	return( false );
}

//---------------------------------------------------------
inline bool CSG_PRQuadTree::_Radius_Contains(double x, double y, double r, int iQuadrant, const TSG_Point &p)	const
{
	return( _Quadrant_Contains(x, y, iQuadrant, p) && _Radius_Contains(x, y, r, p) );
}

//---------------------------------------------------------
inline bool CSG_PRQuadTree::_Quadrant_Intersects(double x, double y, int iQuadrant, CSG_PRQuadTree_Item *pItem)	const
{
	switch( iQuadrant )
	{
	case 0:	return( x <  pItem->Get_xMax() && y <  pItem->Get_yMax() );	// lower left
	case 1:	return( x <  pItem->Get_xMax() && y >= pItem->Get_yMin() );	// upper left
	case 2:	return( x >= pItem->Get_xMin() && y >= pItem->Get_yMin() );	// upper right
	case 3:	return( x >= pItem->Get_xMin() && y <  pItem->Get_yMax() );	// lower right
	}

	return( true );
}

//---------------------------------------------------------
inline bool CSG_PRQuadTree::_Radius_Intersects(double x, double y, double r, CSG_PRQuadTree_Item *pItem)	const
{
	if( r <= 0.0 )
	{
		return( true );
	}

	if(	pItem->Get_xMax() < x - r || pItem->Get_xMin() > x + r
	||	pItem->Get_yMax() < y - r || pItem->Get_yMin() > y + r )
	{
		return( false );
	}

	if( (pItem->Get_xMin() <= x && x <= pItem->Get_xMax())
	||	(pItem->Get_yMin() <= y && y <= pItem->Get_yMax()) )
	{
		return( true );
	}

	TSG_Point	p;

	if( pItem->Get_xMax() < x )
	{
		p.x	= pItem->Get_xMax();
		p.y	= pItem->Get_yMax() < y ? pItem->Get_yMax() : pItem->Get_yMin();
	}
	else // if( pItem->Get_xMin() >= x )
	{
		p.x	= pItem->Get_xMin();
		p.y	= pItem->Get_yMax() < y ? pItem->Get_yMax() : pItem->Get_yMin();
	}

	return( _Radius_Contains(x, y, r, p) );
}

//---------------------------------------------------------
inline bool CSG_PRQuadTree::_Radius_Intersects(double x, double y, double r, int iQuadrant, CSG_PRQuadTree_Item *pItem)	const
{
	return( _Quadrant_Intersects(x, y, iQuadrant, pItem) && _Radius_Intersects(x, y, r, pItem) );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
CSG_PRQuadTree_Leaf * CSG_PRQuadTree::Get_Nearest_Leaf(const TSG_Point &p, double &Distance)	const
{
	return( Get_Nearest_Leaf(p.x, p.y, Distance) );
}

CSG_PRQuadTree_Leaf * CSG_PRQuadTree::Get_Nearest_Leaf(double x, double y, double &Distance)	const
{
	return( _Get_Nearest_Point(m_pRoot, x, y, Distance = -1) );
}

//---------------------------------------------------------
bool CSG_PRQuadTree::Get_Nearest_Point(const TSG_Point &p, TSG_Point &Point, double &Value, double &Distance)	const
{
	return( Get_Nearest_Point(p.x, p.y, Point, Value, Distance) );
}

bool CSG_PRQuadTree::Get_Nearest_Point(double x, double y, TSG_Point &Point, double &Value, double &Distance)	const
{
	CSG_PRQuadTree_Leaf	*pLeaf	= _Get_Nearest_Point(m_pRoot, x, y, Distance = -1);

	if( pLeaf )
	{
		Point.x		= pLeaf->Get_X();
		Point.y		= pLeaf->Get_Y();
		Value		= pLeaf->Get_Z();

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
CSG_PRQuadTree_Leaf	* CSG_PRQuadTree::_Get_Nearest_Point(CSG_PRQuadTree_Item *pItem, double x, double y, double &Distance)	const
{
	CSG_PRQuadTree_Leaf	*pLeaf, *pNearest	= NULL;

	if( pItem )
	{
		if( pItem->is_Leaf() )
		{
			pLeaf	= (CSG_PRQuadTree_Leaf *)pItem;

			double	d	= SG_Get_Distance(x, y, pLeaf->Get_X(), pLeaf->Get_Y(), m_bPolar);

			if( Distance < 0.0 || Distance > d )
			{
				Distance	= d;
				pNearest	= pLeaf;
			}
		}
		else // if( pItem->is_Node() )
		{
			int	i;

			if( pItem->Contains(x, y) )
			{
				for(i=0; i<4; i++)
				{
					CSG_PRQuadTree_Item	*pChild	= ((CSG_PRQuadTree_Node *)pItem)->Get_Child(i);

					if( pChild && pChild->Contains(x, y) && (pLeaf = _Get_Nearest_Point(pChild, x, y, Distance)) != NULL )
					{
						pNearest	= pLeaf;
					}
				}
			}

			for(i=0; i<4; i++)
			{
				CSG_PRQuadTree_Item	*pChild	= ((CSG_PRQuadTree_Node *)pItem)->Get_Child(i);

				if( pChild && pChild->Contains(x, y) == false && (Distance < 0.0
				    || (  Distance > (x < pChild->Get_xCenter() ? pChild->Get_xMin() - x : x - pChild->Get_xMax())
				       && Distance > (y < pChild->Get_yCenter() ? pChild->Get_yMin() - y : y - pChild->Get_yMax()) ))
				&&  (pLeaf = _Get_Nearest_Point(pChild, x, y, Distance)) != NULL )
				{
					pNearest	= pLeaf;
				}
			}
		}
	}

	return( pNearest );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
inline CSG_PRQuadTree::TLeaf * CSG_PRQuadTree::_Get_Selected(const CSG_Array &Selection, size_t i)	const
{
	return( (TLeaf *)Selection.Get_Entry(i) );
}

//---------------------------------------------------------
inline bool CSG_PRQuadTree::_Add_Selected(CSG_Array &Selection, CSG_PRQuadTree_Leaf *pLeaf, double Distance)	const
{
	if( Selection.Inc_Array() )
	{
		TLeaf	*pL		= (TLeaf *)Selection.Get_Entry(Selection.Get_Size() - 1);

		pL->pLeaf		= pLeaf;
		pL->Distance	= Distance;

		return( true );
	}

	return( false );
}

//---------------------------------------------------------
inline bool CSG_PRQuadTree::_Set_Selected(CSG_Array &Selection, size_t i, CSG_PRQuadTree_Leaf *pLeaf, double Distance)	const
{
	TLeaf	*pL		= (TLeaf *)Selection.Get_Entry(i);

	if( pL )
	{
		pL->pLeaf		= pLeaf;
		pL->Distance	= Distance;

		return( true );
	}

	return( false );
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
size_t CSG_PRQuadTree::Select_Nearest_Points(const TSG_Point &p, size_t maxPoints, double Radius, int iQuadrant)
{
	return( Select_Nearest_Points(p.x, p.y, maxPoints, Radius, iQuadrant) );
}

//---------------------------------------------------------
size_t CSG_PRQuadTree::Select_Nearest_Points(double x, double y, size_t maxPoints, double Radius, int iQuadrant)
{
	return( _Select_Nearest_Points(m_Selection, x, y, maxPoints, Radius, iQuadrant) );
}

//---------------------------------------------------------
size_t CSG_PRQuadTree::_Select_Nearest_Points(CSG_Array &Selection, double x, double y, size_t maxPoints, double Radius, int iQuadrant)	const
{
	if( Selection.Get_Value_Size() != sizeof(TLeaf) )
	{
		Selection.Create(sizeof(TLeaf), 0, TSG_Array_Growth::SG_ARRAY_GROWTH_3);
	}
	else
	{
		Selection.Set_Array(0, false);
	}

	if( m_pRoot )
	{
		double	Distance;

		if( maxPoints < 1 )
		{
			maxPoints	= m_nPoints;
		}

		if( iQuadrant != 4 )
		{
			_Select_Nearest_Points(Selection, m_pRoot, x, y, Distance = 0.0, Radius, maxPoints, iQuadrant);
		}
		else // if( iQuadrant == 4 )	// quadrant-wise search
		{
			for(iQuadrant=0; iQuadrant<4; iQuadrant++)
			{
				_Select_Nearest_Points(Selection, m_pRoot, x, y, Distance = 0.0, Radius, maxPoints, iQuadrant);
			}
		}
	}

	return( Selection.Get_Size() );
}

//---------------------------------------------------------
void CSG_PRQuadTree::_Select_Nearest_Points(CSG_Array &Selection, CSG_PRQuadTree_Item *pItem, double x, double y, double &Distance, double Radius, size_t maxPoints, int iQuadrant)	const
{
	//-----------------------------------------------------
	if( pItem->is_Leaf() )
	{
		CSG_PRQuadTree_Leaf	*pLeaf	= (CSG_PRQuadTree_Leaf *)pItem;

		if( _Quadrant_Contains(x, y, iQuadrant, pLeaf->Get_Point()) == false )
		{
			return;
		}
		
		double d = SG_Get_Distance(x, y, pLeaf->Get_X(), pLeaf->Get_Y(), m_bPolar);

		if( Radius > 0.0 && Radius < d )
		{
			return;
		}

		//-------------------------------------------------
		if( Selection.Get_uSize() < maxPoints )
		{
			if( Distance < d )
			{
				Distance = d;
			}

			_Add_Selected(Selection, pLeaf, d);
		}
		else if( d < Distance )
		{
			for(size_t i=0; i<Selection.Get_uSize(); i++)
			{
				if( Distance <= _Get_Selected(Selection, i)->Distance )
				{
					_Set_Selected(Selection, i, pLeaf, d);

					break;
				}
			}

			Distance = d;

			for(size_t i=0; i<maxPoints; i++)
			{
				if( Distance < _Get_Selected(Selection, i)->Distance )
				{
					Distance = _Get_Selected(Selection, i)->Distance;
				}
			}
		}
	}

	//-----------------------------------------------------
	else // if( pItem->is_Node() )
	{
		int	i;

		CSG_PRQuadTree_Item	*pChild;

		for(i=0; i<4; i++)
		{
			if( (pChild = ((CSG_PRQuadTree_Node *)pItem)->Get_Child(i)) != NULL && pChild->Contains(x, y) == true )
			{
				_Select_Nearest_Points(Selection, pChild, x, y, Distance, Radius, maxPoints, iQuadrant);
			}
		}

		for(i=0; i<4; i++)
		{
			if( (pChild = ((CSG_PRQuadTree_Node *)pItem)->Get_Child(i)) != NULL && pChild->Contains(x, y) == false )
			{
				if( _Radius_Intersects(x, y, Radius, iQuadrant, pChild) )
				{
					if( Get_Selected_Count() < maxPoints
					||	(	Distance > (x < pChild->Get_xCenter() ? pChild->Get_xMin() - x : x - pChild->Get_xMax())
						&&	Distance > (y < pChild->Get_yCenter() ? pChild->Get_yMin() - y : y - pChild->Get_yMax())	) )
					{
						_Select_Nearest_Points(Selection, pChild, x, y, Distance, Radius, maxPoints, iQuadrant);
					}
				}
			}
		}
	}
}


///////////////////////////////////////////////////////////
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
size_t CSG_PRQuadTree::Get_Nearest_Points(CSG_Points_3D &Points, const TSG_Point &p, size_t maxPoints, double Radius, int iQuadrant)	const
{
	return( Get_Nearest_Points(Points, p.x, p.y, maxPoints, Radius, iQuadrant) );
}

//---------------------------------------------------------
size_t CSG_PRQuadTree::Get_Nearest_Points(CSG_Points_3D &Points, double x, double y, size_t maxPoints, double Radius, int iQuadrant)	const
{
	CSG_Array	Selection;

	_Select_Nearest_Points(Selection, x, y, maxPoints, Radius, iQuadrant);

	Points.Clear();

	for(size_t i=0; i<Selection.Get_uSize(); i++)
	{
		CSG_PRQuadTree_Leaf	*pLeaf	= _Get_Selected(Selection, i)->pLeaf;

		Points.Add(pLeaf->Get_X(), pLeaf->Get_Y(), pLeaf->Get_Z());
	}

	return( Points.Get_Count() );
}


///////////////////////////////////////////////////////////
//                                                       //
//                                                       //
//                                                       //
///////////////////////////////////////////////////////////

//---------------------------------------------------------
