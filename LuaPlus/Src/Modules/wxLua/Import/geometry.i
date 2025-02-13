/////////////////////////////////////////////////////////////////////////////
// Purpose:     wxPoint2DInt, wxRect2DInt and other classes from wx/geometry.h
// Author:      J Winwood
// Created:     14/11/2001
// Copyright:   (c) 2001-2002 Lomtick Software. All rights reserved.
// Licence:     wxWindows licence
/////////////////////////////////////////////////////////////////////////////

%if wxLUA_USE_Geometry

%typedef wxInt32 int
%typedef wxDouble double

%enum wxOutCode
    wxInside
    wxOutLeft
    wxOutRight
    wxOutTop
    wxOutBottom
%end

///////////////////////////////////////////////////////////////////////////////
// wxRect2DDouble

%include "wx/geometry.h"
%class %delete %noclassinfo %encapsulate wxRect2DDouble
    //wxRect2DDouble()
    wxRect2DDouble(wxDouble x=0, wxDouble y=0, wxDouble w=0, wxDouble h=0)
    wxPoint2DDouble GetPosition()
    wxSize GetSize()
    wxDouble GetLeft() const
    void SetLeft( wxDouble n )
    void MoveLeftTo( wxDouble n )
    wxDouble GetTop() const
    void SetTop( wxDouble n )
    void MoveTopTo( wxDouble n )
    wxDouble GetBottom() const
    void SetBottom( wxDouble n )
    void MoveBottomTo( wxDouble n )
    wxDouble GetRight() const
    void SetRight( wxDouble n )
    void MoveRightTo( wxDouble n )
    wxPoint2DDouble GetLeftTop() const
    void SetLeftTop( const wxPoint2DDouble &pt )
    void MoveLeftTopTo( const wxPoint2DDouble &pt )
    wxPoint2DDouble GetLeftBottom() const
    void SetLeftBottom( const wxPoint2DDouble &pt )
    void MoveLeftBottomTo( const wxPoint2DDouble &pt )
    wxPoint2DDouble GetRightTop() const
    void SetRightTop( const wxPoint2DDouble &pt )
    void MoveRightTopTo( const wxPoint2DDouble &pt )
    wxPoint2DDouble GetRightBottom() const
    void SetRightBottom( const wxPoint2DDouble &pt )
    void MoveRightBottomTo( const wxPoint2DDouble &pt )
    wxPoint2DDouble GetCentre() const
    void SetCentre( const wxPoint2DDouble &pt )
    void MoveCentreTo( const wxPoint2DDouble &pt )
    wxOutCode GetOutCode( const wxPoint2DDouble &pt ) const
    %rename ContainsPoint bool Contains( const wxPoint2DDouble &pt ) const
    %rename ContainsRect  bool Contains( const wxRect2DDouble &rect ) const
    bool IsEmpty() const
    bool HaveEqualSize( const wxRect2DDouble &rect ) const
    //void Inset( wxDouble x, wxDouble y )
    void Inset( wxDouble left, wxDouble top, wxDouble right, wxDouble bottom  )
    void Offset( const wxPoint2DDouble &pt )
    void ConstrainTo( const wxRect2DDouble &rect )
    wxPoint2DDouble Interpolate( wxInt32 widthfactor , wxInt32 heightfactor )
    //static void Intersect( const wxRect2DDouble &src1 , const wxRect2DDouble &src2 , wxRect2DDouble *dest )
    void Intersect( const wxRect2DDouble &otherRect )
    wxRect2DDouble CreateIntersection( const wxRect2DDouble &otherRect ) const
    bool Intersects( const wxRect2DDouble &rect ) const
    //static void Union( const wxRect2DDouble &src1 , const wxRect2DDouble &src2 , wxRect2DDouble *dest )
    void Union( const wxRect2DDouble &otherRect )
    //void Union( const wxPoint2DDouble &pt )
    wxRect2DDouble CreateUnion( const wxRect2DDouble &otherRect ) const
    void Scale( wxDouble f )
    //void Scale( wxInt32 num , wxInt32 denum )
    %rename X %member wxDouble m_x
    %member wxDouble m_y
    %rename Width %member wxDouble m_width
    %member wxDouble m_height
%endclass

///////////////////////////////////////////////////////////////////////////////
// wxPoint2DDouble

%include "wx/geometry.h"
%class %delete %noclassinfo %encapsulate wxPoint2DDouble
    //wxPoint2DDouble()
    wxPoint2DDouble( wxDouble x=0, wxDouble y=0 )
    //%constructor wxPoint2DDouble( const wxPoint2DDouble &pt )
    //%constructor wxPoint2DDouble( const wxPoint2DInt &pt )
    //%constructor wxPoint2DDouble( const wxPoint &pt )
    //void GetFloor( wxInt32 *x , wxInt32 *y ) const
    //void GetRounded( wxInt32 *x , wxInt32 *y ) const
    wxDouble GetVectorLength() const
    wxDouble GetVectorAngle() const
    void SetVectorLength( wxDouble length )
    void SetVectorAngle( wxDouble degrees )
    //void SetPolarCoordinates( wxDouble angle , wxDouble length )
    //void Normalize()
    wxDouble GetDistance( const wxPoint2DDouble &pt ) const
    wxDouble GetDistanceSquare( const wxPoint2DDouble &pt ) const
    wxDouble GetDotProduct( const wxPoint2DDouble &vec ) const
    wxDouble GetCrossProduct( const wxPoint2DDouble &vec ) const

    //wxPoint2DDouble operator-()
    //wxPoint2DDouble& operator=(const wxPoint2DDouble& pt)
    //wxPoint2DDouble& operator+=(const wxPoint2DDouble& pt)
    //wxPoint2DDouble& operator-=(const wxPoint2DDouble& pt)
    //wxPoint2DDouble& operator*=(const wxPoint2DDouble& pt)
    //wxPoint2DDouble& operator*=(wxDouble n)
    //wxPoint2DDouble& operator*=(wxInt32 n)
    //wxPoint2DDouble& operator/=(const wxPoint2DDouble& pt)
    //wxPoint2DDouble& operator/=(wxDouble n)
    //wxPoint2DDouble& operator/=(wxInt32 n)
    //bool operator==(const wxPoint2DDouble& pt) const
    //bool operator!=(const wxPoint2DDouble& pt) const
    //%member wxDouble m_x
    //%member wxDouble m_y
%endclass

///////////////////////////////////////////////////////////////////////////////
// wxTransform2D

//%include "wx/geometry.h"
//%class %noclassinfo wxTransform2D
//    virtual void                    Transform( wxPoint2DInt* pt )const  = 0
//    virtual void                    Transform( wxRect2DInt* r ) const
//    virtual wxPoint2DInt    Transform( const wxPoint2DInt &pt ) const
//    virtual wxRect2DInt        Transform( const wxRect2DInt &r ) const
//    virtual void                    InverseTransform( wxPoint2DInt* pt ) const  = 0
//    virtual void                    InverseTransform( wxRect2DInt* r ) const
//    virtual wxPoint2DInt    InverseTransform( const wxPoint2DInt &pt ) const
//    virtual wxRect2DInt        InverseTransform( const wxRect2DInt &r ) const
//    void    wxTransform2D::Transform( wxRect2DInt* r ) const
//    wxPoint2DInt    wxTransform2D::Transform( const wxPoint2DInt &pt ) const
//    wxRect2DInt     wxTransform2D::Transform( const wxRect2DInt &r ) const
//    void    wxTransform2D::InverseTransform( wxRect2DInt* r ) const
//    wxPoint2DInt    wxTransform2D::InverseTransform( const wxPoint2DInt &pt ) const
//    wxRect2DInt     wxTransform2D::InverseTransform( const wxRect2DInt &r ) const
//%endclass

///////////////////////////////////////////////////////////////////////////////
// wxRect2DInt

%include "wx/geometry.h"
%class %delete %noclassinfo %encapsulate wxRect2DInt
    //wxRect2DInt()
    wxRect2DInt(wxInt32 x=0, wxInt32 y=0, wxInt32 w=0, wxInt32 h=0)
    %constructor wxRect2DIntFromwxRect( const wxRect& r )
    %constructor wxRect2DIntFromPoints(const wxPoint2DInt& topLeft, const wxPoint2DInt& bottomRight)
    //%constructor wxRect2DInt(const wxPoint2DInt& pos, const wxSize& size)
    %constructor wxRect2DIntFromwxRect2DInt(const wxRect2DInt& rect)
    wxPoint2DInt GetPosition()
    wxSize GetSize()
    wxInt32 GetLeft() const
    void SetLeft( wxInt32 n )
    void MoveLeftTo( wxInt32 n )
    wxInt32 GetTop() const
    void SetTop( wxInt32 n )
    void MoveTopTo( wxInt32 n )
    wxInt32 GetBottom() const
    void SetBottom( wxInt32 n )
    void MoveBottomTo( wxInt32 n )
    wxInt32 GetRight() const
    void SetRight( wxInt32 n )
    void MoveRightTo( wxInt32 n )
    wxPoint2DInt GetLeftTop() const
    void SetLeftTop( const wxPoint2DInt &pt )
    void MoveLeftTopTo( const wxPoint2DInt &pt )
    wxPoint2DInt GetLeftBottom() const
    void SetLeftBottom( const wxPoint2DInt &pt )
    void MoveLeftBottomTo( const wxPoint2DInt &pt )
    wxPoint2DInt GetRightTop() const
    void SetRightTop( const wxPoint2DInt &pt )
    void MoveRightTopTo( const wxPoint2DInt &pt )
    wxPoint2DInt GetRightBottom() const
    void SetRightBottom( const wxPoint2DInt &pt )
    void MoveRightBottomTo( const wxPoint2DInt &pt )
    wxPoint2DInt GetCentre() const
    void SetCentre( const wxPoint2DInt &pt )
    void MoveCentreTo( const wxPoint2DInt &pt )
    wxOutCode GetOutCode( const wxPoint2DInt &pt ) const
    wxOutCode GetOutcode( const wxPoint2DInt &pt ) const
    %rename ContainsPoint bool Contains( const wxPoint2DInt &pt ) const
    %rename ContainsRect  bool Contains( const wxRect2DInt &rect ) const
    bool IsEmpty() const
    bool HaveEqualSize( const wxRect2DInt &rect ) const
    //void Inset( wxInt32 x , wxInt32 y )
    void Inset( wxInt32 left , wxInt32 top ,wxInt32 right , wxInt32 bottom  )
    void Offset( const wxPoint2DInt &pt )
    void ConstrainTo( const wxRect2DInt &rect )
    wxPoint2DInt Interpolate( wxInt32 widthfactor , wxInt32 heightfactor )
    //static void Intersect( const wxRect2DInt &src1 , const wxRect2DInt &src2 , wxRect2DInt *dest )
    void Intersect( const wxRect2DInt &otherRect )
    wxRect2DInt CreateIntersection( const wxRect2DInt &otherRect ) const
    bool Intersects( const wxRect2DInt &rect ) const
    //static void Union( const wxRect2DInt &src1 , const wxRect2DInt &src2 , wxRect2DInt *dest )
    void Union( const wxRect2DInt &otherRect )
    //void Union( const wxPoint2DInt &pt )
    wxRect2DInt CreateUnion( const wxRect2DInt &otherRect ) const
    void Scale( wxInt32 f )
    //void Scale( wxInt32 num , wxInt32 denum )

    //void WriteTo( wxDataOutputStream &stream ) const
    //void ReadFrom( wxDataInputStream &stream )

    //%member wxInt32 m_x
    //%member wxInt32 m_y
    //%member wxInt32 m_width
    //%member wxInt32 m_height
    //wxRect2DInt& operator = (const wxRect2DInt& rect)
    //bool operator == (const wxRect2DInt& rect) const
    //bool operator != (const wxRect2DInt& rect) const
%endclass

///////////////////////////////////////////////////////////////////////////////
// wxPoint2DInt

%include "wx/geometry.h"
%class %delete %noclassinfo %encapsulate wxPoint2DInt
    //wxPoint2DInt()
    %constructor wxPoint2DInt( wxInt32 x=0, wxInt32 y=0 )
    //%constructor wxPoint2DInt( const wxPoint2DInt &pt )
    //%constructor wxPoint2DInt( const wxPoint &pt )
    //void GetFloor( wxInt32 *x , wxInt32 *y ) const
    //void GetRounded( wxInt32 *x , wxInt32 *y ) const
    wxDouble GetVectorLength() const
    wxDouble GetVectorAngle() const
    void SetVectorLength( wxDouble length )
    void SetVectorAngle( wxDouble degrees )
    //void SetPolarCoordinates( wxInt32 angle , wxInt32 length )
    //void Normalize()
    wxDouble GetDistance( const wxPoint2DInt &pt ) const
    wxDouble GetDistanceSquare( const wxPoint2DInt &pt ) const
    wxInt32 GetDotProduct( const wxPoint2DInt &vec ) const
    wxInt32 GetCrossProduct( const wxPoint2DInt &vec ) const

    //void WriteTo( wxDataOutputStream &stream ) const
    //void ReadFrom( wxDataInputStream &stream )
    //%member wxInt32 m_x
    //%member wxInt32 m_y
    //wxPoint2DInt operator-()
    //wxPoint2DInt& operator=(const wxPoint2DInt& pt)
    //wxPoint2DInt& operator+=(const wxPoint2DInt& pt)
    //wxPoint2DInt& operator-=(const wxPoint2DInt& pt)
    //wxPoint2DInt& operator*=(const wxPoint2DInt& pt)
    //wxPoint2DInt& operator*=(wxDouble n)
    //wxPoint2DInt& operator*=(wxInt32 n)
    //wxPoint2DInt& operator/=(const wxPoint2DInt& pt)
    //wxPoint2DInt& operator/=(wxDouble n)
    //wxPoint2DInt& operator/=(wxInt32 n)
    //operator wxPoint() const
    //bool operator==(const wxPoint2DInt& pt) const
    //bool operator!=(const wxPoint2DInt& pt) const
%endclass

%endif wxLUA_USE_Geometry
