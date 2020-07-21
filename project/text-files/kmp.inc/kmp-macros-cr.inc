
#
###############################################################################
# Macros

@macro itembox
    #-------------------------------------------------------------------
    # This macro helps to place N itemboxes in a row between
    # 2 points with optimal height.
    #-------------------------------------------------------------------
    # Syntax:
    #	@:ITEMBOX(fall,1,pos1)
    #	@:ITEMBOX(fall,n,pos1,pos2)
    #	@:ITEMBOX(fall,n,pos1,pos2,factor)
    #	@:ITEMBOX(fall,n,pos1,pos2,factor,second)
    #	@:ITEMBOX(fall,n,pos1,pos2,factor,second,itemtype)
    #-------------------------------------------------------------------
    # Parameters:
    #	FALL	: If >0: Raise the box by FALL and let it fall to
    #		: the road to get the optimal height.
    #	N	: If >0: Create N itemboxes between POS1 and POS2.
    #	POS1	: Position of first itembox.
    #	POS2	: Position of last itembox.
    #	FACTOR	: If set and >0.01: Create a line from POS1 to POS2.
    #		: Enlarge the line by FACTOR in both directions.
    #		: Then use the new points as new POS1 and POS2.
    #   SECOND	: If set and >0: Move each second itembox into a second
    #		: row. The distance between both rows is SECOND units.
    #   ITEMTYPE: Use this value for player and enemy items.
    #-------------------------------------------------------------------
    # Examples:
    #	@:itembox( 200, 1, point )
    #	@:itembox(   0, 3, point1, point2, 0.9 )
    #	@:itembox( 500, 8, point1, point2, 1, 250, ITEM$3GREEN )
    #	@:itembox( 200, 4, v(2000,1000,3000), v(3000,1000,5000) )
    #-------------------------------------------------------------------

    @param fall n p1 p2 factor second itemtype
    @pdef fall = fall > 0 ? fall : 0
    @if isScalar(factor) && factor > 0.01
	@pdef a = p1, b = p2, f = (1.0-factor)/2
	@pdef p1 = pos(f,a,b)
	@pdef p2 = pos(f,b,a)
    @endif
    @pdef itemtype = int(itemtype)
    @if n == 1
	@pdef p = fall ? kcl$fall(p1+vy(fall),200,KCL$DRIVE) : p1
	0 o$itembox p 0 itemtype itemtype 0 -1  0 v3(0) 0 0 0 0 0x3f  v3(1)
    @else
	@pdef second = isScalar(second) && second > 0
		> ? hrot(vz(second),hdir(p1,p2)+90) : v3(0)
	@pdef n = n - 1.0
	@for i=0;n
	    @pdef p = pos(i/n,p1,p2)
	    @doif(i&1)  @pdef p = p + second
	    @doif(fall) @pdef p = kcl$fall(p+vy(fall),200,KCL$DRIVE)
	    0 o$itembox p 0 itemtype itemtype 0 -1  0 v3(0) 0 0 0 0 0x3f  v3(1)
	@endfor
    @endif
@endmacro
