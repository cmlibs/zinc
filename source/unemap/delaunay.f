      SUBROUTINE ENTERS(NAME,*)

      IMPLICIT NONE
!     Parameter List
      CHARACTER NAME*(*)

      RETURN
      END


      SUBROUTINE EXITS(NAME)

      IMPLICIT NONE
!     Parameter List
      CHARACTER NAME*(*)

      RETURN
      END


      SUBROUTINE ERRORS(ROUTINE_NAME,ERROR)

      IMPLICIT NONE
!     Parameter List
      CHARACTER ERROR*(*),ROUTINE_NAME*(*)

      RETURN
      END


      SUBROUTINE SPHERE_PROJECT(RAW_XYZ,XYZ,IERROR,ERROR,*)

C#### Subroutine: SPHERE_PROJECT
C###  Description:
C###    Projects a point (RAW_XYZ) onto the surface of a unit sphere
C###    (XYZ).  IERROR not zero indicates an error.

C***  Created by: David Bullivant, January 2002
C***  Last modified: 14 February 2002

      IMPLICIT NONE
!     Parameter List
      INTEGER IERROR
      REAL*8 RAW_XYZ(3),XYZ(3)
      REAL*8 RADIUS
      CHARACTER ERROR*(*)

      CALL ENTERS('SPHERE_PROJECT',*9999)

      IERROR=0
      RADIUS=SQRT(RAW_XYZ(1)*RAW_XYZ(1)+RAW_XYZ(2)*RAW_XYZ(2)+
     '  RAW_XYZ(3)*RAW_XYZ(3))
      IF (RADIUS.GT.0.d0) THEN
        XYZ(1)=RAW_XYZ(1)/RADIUS
        XYZ(2)=RAW_XYZ(2)/RADIUS
        XYZ(3)=RAW_XYZ(3)/RADIUS
      ELSE
        XYZ(1)=1.d0
        XYZ(2)=0.d0
        XYZ(3)=0.d0
        IERROR=2
      ENDIF

      CALL EXITS('SPHERE_PROJECT')
      RETURN
 9999 CALL ERRORS('SPHERE_PROJECT',ERROR)
      CALL EXITS('SPHERE_PROJECT')
      RETURN 1
      END


      SUBROUTINE SPHERE_CALC_DIST(XYZ_1,XYZ_2,DISTANCE,IERROR,
     '  ERROR,*)

C#### Subroutine: SPHERE_CALC_DIST
C###  Description:
C###    Calculates the DISTANCE between two points, XYZ_1 and XYZ_2, 
C###    on the surface of a unit sphere.  The DISTANCE between them is
C###    the shorter distance along the great circle through them.  
C###    IERROR not zero indicates an error.

C***  Created by: David Bullivant, January 2002
C***  Last modified: 14 February 2002

      IMPLICIT NONE
!     Parameter List
      INTEGER IERROR
      REAL*8 DISTANCE,XYZ_1(3),XYZ_2(3)
      CHARACTER ERROR*(*)

      CALL ENTERS('SPHERE_CALC_DIST',*9999)

      IERROR=0
C     dot product
      DISTANCE=XYZ_1(1)*XYZ_2(1)+XYZ_1(2)*XYZ_2(2)+XYZ_1(3)*XYZ_2(3)
      DISTANCE=ACOS(DISTANCE)
      IF (DISTANCE.LT.0.d0) THEN
        DISTANCE= -DISTANCE
      ENDIF

      CALL EXITS('SPHERE_CALC_DIST')
      RETURN
 9999 CALL ERRORS('SPHERE_CALC_DIST',ERROR)
      CALL EXITS('SPHERE_CALC_DIST')
      RETURN 1
      END


      SUBROUTINE SPHERE_CALC_CIRCUM(XYZ_1,XYZ_2,XYZ_3,XYZ_CENTRE,
     '  IERROR,ERROR,*)

C#### Subroutine: SPHERE_CALC_CIRCUM
C###  Description:
C###  Calculates the circumcentre (XYZ_CENTRE) of three points 
C###   (XYZ_1, XYZ_2 and XYZ_3) on the surface of a sphere.  There are 
C###   two choices for the centre.  The straight line in 3-space 
C###   between the two choices goes through the centre of the sphere. 
C###   One is chosen based on the order of the vertices.  IERROR not
C###   zero indicates an error.

C***  Created by: David Bullivant, January 2002
C***  Last modified: 14 February 2002

      IMPLICIT NONE
!     Parameter List
      INTEGER IERROR
      REAL*8 XYZ_CENTRE(3),XYZ_1(3),XYZ_2(3),XYZ_3(3)
      CHARACTER ERROR*(*)
!     Local Variables
      REAL*8 XYZ(3),X12,X13,Y12,Y13,Z12,Z13

      CALL ENTERS('SPHERE_CALC_CIRCUM',*9999)

      IERROR=0
      X12=XYZ_1(1)-XYZ_2(1)
      Y12=XYZ_1(2)-XYZ_2(2)
      Z12=XYZ_1(3)-XYZ_2(3)
      X13=XYZ_1(1)-XYZ_3(1)
      Y13=XYZ_1(2)-XYZ_3(2)
      Z13=XYZ_1(3)-XYZ_3(3)
      XYZ(1)=Y12*Z13-Z12*Y13
      XYZ(2)=Z12*X13-X12*Z13
      XYZ(3)=X12*Y13-Y12*X13
      CALL SPHERE_PROJECT(XYZ,XYZ_CENTRE,IERROR,ERROR,*9999)

      CALL EXITS('SPHERE_CALC_CIRCUM')
      RETURN
 9999 CALL ERRORS('SPHERE_CALC_CIRCUM',ERROR)
      CALL EXITS('SPHERE_CALC_CIRCUM')
      RETURN 1
      END


C     for CMISS
C      SUBROUTINE SPHERE_DELAUNAY(VERTICES_XYZ,
C     '  TRIANGLES,RWORKING,IERROR,ERROR,*)
      SUBROUTINE SPHERE_DELAUNAY(N_VERTICES,VERTICES_XYZ,MAX_TRIANGLES,
     '  TRIANGLES,N_TRIANGLES,RWORKING,IERROR)

C#### Subroutine: SPHERE_DELAUNAY
C###  Description: 
C###    Calculates the Delaunay triangulation of the
C###    vertices on a unit sphere.

C***  Created by: David Bullivant, January 2002
C***  Last modified: 10 April 2002

C***  N_VERTICES is the number of vertices to be triangulated
C***  VERTICES_XYZ is an array of length 3*N_VERTICES, containing the
C***    x,y,z coordinates of the vertices (X1,Y1,Z1, X2,Y2,Z2, ...).
C***    After successful completion, the vertices will be projected onto
C***    the unit sphere
C***  MAX_TRIANGLES is the number of triangles that can be held in
C***    TRIANGLES
C***  TRIANGLES.  After successful completion, TRIANGLES will contain
C***    the vertex numbers for the N_TRIANGLES.  It needs to be at least
C***    3*MAX_TRIANGLES long
C***  N_TRIANGLES.  After successful completion, this is the number of
C***    triangles in the triangulation
C***  RWORKING a real working array that is at least 4*MAX_TRIANGLES
C***    long
C***  IERROR not zero indicates an error
C***    IERROR=1 more than MAX_TRIANGLES
C***    IERROR=2 error in SPHERE_PROJECT
C***    IERROR=3 invalid arguments
C***    IERROR=4 could not find initial tetrahedron

      IMPLICIT NONE
C     for CMISS
C      INCLUDE 'cmiss$reference:genmesh.cmn'
      INTEGER MAX_TRIANGLES,N_TRIANGLES,N_VERTICES
!     Parameter List
      INTEGER IERROR,TRIANGLES(3*MAX_TRIANGLES)
      REAL*8 RWORKING(4*MAX_TRIANGLES),VERTICES_XYZ(3*N_VERTICES)
C     for CMISS
C      CHARACTER ERROR*(*)
      CHARACTER*10 ERROR
!     Local Variables
      INTEGER I,INITIAL_TETRAHEDRON(4),ITEMP,J,K,L,M,N,N_ADJACENT_LINES,
     '  N1,N2
      REAL*8 LENGTH,PI,RTEMP,XYZ(3),X1,X2,X3,X4,Y1,Y2,Y3,Y4,Z1,Z2,Z3,Z4
!     Constants
      REAL*8 TOLERANCE
      PARAMETER (TOLERANCE=1.d-4)

      CALL ENTERS('SPHERE_DELAUNAY',*9999)

      PI=4.d0*ATAN(1.d0)
C     normalize vertices
      I=1
      DO J=1,N_VERTICES
        CALL SPHERE_PROJECT(VERTICES_XYZ(I),XYZ,IERROR,ERROR,*9999)
        VERTICES_XYZ(I)=XYZ(1)
        I=I+1
        VERTICES_XYZ(I)=XYZ(2)
        I=I+1
        VERTICES_XYZ(I)=XYZ(3)
        I=I+1
      ENDDO !J
      IERROR=0
      IF (4.LE.N_VERTICES) THEN
        IF (4.LE.MAX_TRIANGLES) THEN
C         find a tetrahedron for the initial triangulation (4 triangles)
C         first corner
          INITIAL_TETRAHEDRON(1)=1
          I=1
C         second corner must be different from first corner and not
C           opposite to first corner
   10     CONTINUE
            I=I+1
            CALL SPHERE_CALC_DIST(VERTICES_XYZ(1),
     '        VERTICES_XYZ(3*I-2),LENGTH,IERROR,ERROR,*9999)
          IF ((0.EQ.IERROR).AND.(I.LT.N_VERTICES-2).AND.
     '      ((LENGTH.LE.TOLERANCE).OR.(LENGTH.GE.PI-TOLERANCE))) GOTO 10
          IF ((0.EQ.IERROR).AND.(TOLERANCE.LT.LENGTH).AND.
     '      (LENGTH.LT.PI-TOLERANCE)) THEN
            INITIAL_TETRAHEDRON(2)=I
C           third corner can't be on the same line as first two corners
            X1=VERTICES_XYZ(1)
            Y1=VERTICES_XYZ(2)
            Z1=VERTICES_XYZ(3)
            J=3*I-2
            X2=VERTICES_XYZ(J)
            Y2=VERTICES_XYZ(J+1)
            Z2=VERTICES_XYZ(J+2)
            X3=Y1*Z2-Z1*Y2
            Y3=Z1*X2-X1*Z2
            Z3=X1*Y2-Y1*X2
   20       CONTINUE
              I=I+1
              J=3*I-2
              X2=VERTICES_XYZ(J)
              Y2=VERTICES_XYZ(J+1)
              Z2=VERTICES_XYZ(J+2)
              X4=Y1*Z2-Z1*Y2
              Y4=Z1*X2-X1*Z2
              Z4=X1*Y2-Y1*X2
              LENGTH=SQRT(X4*X4+Y4*Y4+Z4*Z4)
              RTEMP=SQRT((X3-X4)*(X3-X4)+(Y3-Y4)*(Y3-Y4)+
     '          (Z3-Z4)*(Z3-Z4))
            IF ((I.LT.N_VERTICES-1).AND.
     '        ((LENGTH.LE.TOLERANCE).OR.(RTEMP.LE.TOLERANCE))) GOTO 20
            IF (((LENGTH.GT.TOLERANCE).AND.(RTEMP.GT.TOLERANCE))) THEN
              INITIAL_TETRAHEDRON(3)=I
C             fourth corner can't be in same plane as other 3
              CALL SPHERE_CALC_CIRCUM(
     '          VERTICES_XYZ(3*INITIAL_TETRAHEDRON(1)-2),
     '          VERTICES_XYZ(3*INITIAL_TETRAHEDRON(2)-2),
     '          VERTICES_XYZ(3*INITIAL_TETRAHEDRON(3)-2),XYZ,IERROR,
     '          ERROR,*9999)
              IF (0.EQ.IERROR) THEN
                X1=XYZ(1)
                Y1=XYZ(2)
                Z1=XYZ(3)
   30           CONTINUE
                  I=I+1
C                 make sure that first face is the smaller triangle
                  CALL SPHERE_CALC_CIRCUM(
     '              VERTICES_XYZ(3*INITIAL_TETRAHEDRON(1)-2),
     '              VERTICES_XYZ(3*INITIAL_TETRAHEDRON(2)-2),
     '              VERTICES_XYZ(3*I-2),XYZ,IERROR,ERROR,*9999)
                  IF (0.EQ.IERROR) THEN
                    X2=XYZ(1)
                    Y2=XYZ(2)
                    Z2=XYZ(3)
                    X4=Y1*Z2-Z1*Y2
                    Y4=Z1*X2-X1*Z2
                    Z4=X1*Y2-Y1*X2
                    LENGTH=SQRT(X4*X4+Y4*Y4+Z4*Z4)
                  ENDIF
                IF ((0.EQ.IERROR).AND.(I.LT.N_VERTICES).AND.
     '            (LENGTH.LE.TOLERANCE)) GOTO 30
                IF ((0.EQ.IERROR).AND.(LENGTH.GT.TOLERANCE)) THEN
                  INITIAL_TETRAHEDRON(4)=I
C                 determine which side of the first face the fourth
C                   corner is
                  J=3*I-2
                  K=3*INITIAL_TETRAHEDRON(1)-2
                  X2=VERTICES_XYZ(J)-VERTICES_XYZ(K)
                  Y2=VERTICES_XYZ(J+1)-VERTICES_XYZ(K+1)
                  Z2=VERTICES_XYZ(J+2)-VERTICES_XYZ(K+2)
                  LENGTH=X1*X2+Y1*Y2+Z1*Z2
                  IF (LENGTH.GT.0.D0) THEN
                    J=INITIAL_TETRAHEDRON(2)
                    INITIAL_TETRAHEDRON(2)=INITIAL_TETRAHEDRON(3)
                    INITIAL_TETRAHEDRON(3)=J
                  ENDIF
                ELSE
                  IERROR=4
                ENDIF
              ENDIF
            ELSE
              IERROR=4
            ENDIF
          ELSE
            IF (0.EQ.IERROR) THEN
              IERROR=4
            ENDIF
          ENDIF
          IF (0.EQ.IERROR) THEN
            N_TRIANGLES=4
C           initial triangulation covers the sphere
C           triangle 1
            TRIANGLES(1)=INITIAL_TETRAHEDRON(1)
            TRIANGLES(2)=INITIAL_TETRAHEDRON(2)
            TRIANGLES(3)=INITIAL_TETRAHEDRON(3)
            CALL SPHERE_CALC_CIRCUM(VERTICES_XYZ(3*TRIANGLES(1)-2),
     '        VERTICES_XYZ(3*TRIANGLES(2)-2),
     '        VERTICES_XYZ(3*TRIANGLES(3)-2),RWORKING(1),IERROR,ERROR,
     '        *9999)
            IF (0.EQ.IERROR) THEN
C             Calculate radius of triangle
              CALL SPHERE_CALC_DIST(RWORKING(1),
     '          VERTICES_XYZ(3*TRIANGLES(1)-2),RWORKING(4),IERROR,ERROR,
     '          *9999)
              IF (0.EQ.IERROR) THEN
C               triangle 2
                TRIANGLES(4)=INITIAL_TETRAHEDRON(1)
                TRIANGLES(5)=INITIAL_TETRAHEDRON(4)
                TRIANGLES(6)=INITIAL_TETRAHEDRON(2)
                CALL SPHERE_CALC_CIRCUM(VERTICES_XYZ(3*TRIANGLES(4)-2),
     '            VERTICES_XYZ(3*TRIANGLES(5)-2),
     '            VERTICES_XYZ(3*TRIANGLES(6)-2),RWORKING(5),IERROR,
     '            ERROR,*9999)
                IF (0.EQ.IERROR) THEN
C                 Calculate radius of triangle
                  CALL SPHERE_CALC_DIST(RWORKING(5),
     '              VERTICES_XYZ(3*TRIANGLES(4)-2),RWORKING(8),IERROR,
     '              ERROR,*9999)
                  IF (0.EQ.IERROR) THEN
C                   triangle 3
                    TRIANGLES(7)=INITIAL_TETRAHEDRON(2)
                    TRIANGLES(8)=INITIAL_TETRAHEDRON(4)
                    TRIANGLES(9)=INITIAL_TETRAHEDRON(3)
                    CALL SPHERE_CALC_CIRCUM(
     '                VERTICES_XYZ(3*TRIANGLES(7)-2),
     '                VERTICES_XYZ(3*TRIANGLES(8)-2),
     '                VERTICES_XYZ(3*TRIANGLES(9)-2),RWORKING(9),IERROR,
     '                ERROR,*9999)
                    IF (0.EQ.IERROR) THEN
C                     Calculate radius of triangle
                      CALL SPHERE_CALC_DIST(RWORKING(9),
     '                  VERTICES_XYZ(3*TRIANGLES(7)-2),RWORKING(12),
     '                  IERROR,ERROR,*9999)
                      IF (0.EQ.IERROR) THEN
C                       triangle 4
                        TRIANGLES(10)=INITIAL_TETRAHEDRON(3)
                        TRIANGLES(11)=INITIAL_TETRAHEDRON(4)
                        TRIANGLES(12)=INITIAL_TETRAHEDRON(1)
                        CALL SPHERE_CALC_CIRCUM(
     '                    VERTICES_XYZ(3*TRIANGLES(10)-2),
     '                    VERTICES_XYZ(3*TRIANGLES(11)-2),
     '                    VERTICES_XYZ(3*TRIANGLES(12)-2),RWORKING(13),
     '                    IERROR,ERROR,*9999)
                        IF (0.EQ.IERROR) THEN
C                         Calculate radius of triangle
                          CALL SPHERE_CALC_DIST(RWORKING(13),
     '                      VERTICES_XYZ(3*TRIANGLES(10)-2),
     '                      RWORKING(16),IERROR,ERROR,*9999)
                        ENDIF
                      ENDIF
                    ENDIF
                  ENDIF
                ENDIF
              ENDIF
            ENDIF
            INITIAL_TETRAHEDRON(1)= 3*INITIAL_TETRAHEDRON(1)-2
            INITIAL_TETRAHEDRON(2)= 3*INITIAL_TETRAHEDRON(2)-2
            INITIAL_TETRAHEDRON(3)= 3*INITIAL_TETRAHEDRON(3)-2
            INITIAL_TETRAHEDRON(4)= 3*INITIAL_TETRAHEDRON(4)-2
            I=1
            DO WHILE ((0.EQ.IERROR).AND.(I.LE.3*N_VERTICES))
              IF ((I.NE.INITIAL_TETRAHEDRON(1)).AND.
     '          (I.NE.INITIAL_TETRAHEDRON(2)).AND.
     '          (I.NE.INITIAL_TETRAHEDRON(3)).AND.
     '          (I.NE.INITIAL_TETRAHEDRON(4))) THEN
C               delete the triangles that no longer satisfy the
C                 in-circle criterion
                N_ADJACENT_LINES=0
                J=1
                K=0
                DO WHILE ((0.EQ.IERROR).AND.(J.LE.N_TRIANGLES))
C                 Calculate distance from vertex point to circumcentre
C                   of triangle J
                  CALL SPHERE_CALC_DIST(RWORKING(4*J-3),VERTICES_XYZ(I),
     '              LENGTH,IERROR,ERROR,*9999)
C                 If length.LE.radius, then vertex is within the circle
C                 Triangle will be discarded, replaced by new ones
                  IF (0.EQ.IERROR) THEN
                    IF (LENGTH.LE.RWORKING(4*J)+TOLERANCE) THEN
C                     add to the list of adjacent vertices
C                     use the end of the triangles array as working
C                       storage for the adjacent lines
                      N2=TRIANGLES(3*J)
                      N=1
                      DO WHILE ((0.EQ.IERROR).AND.(N.LE.3))
                        N1=N2
                        N2=TRIANGLES(3*J-3+N)
C                       check for duplicates
                        L=1
                        M=3*MAX_TRIANGLES
                        DO WHILE ((L.LE.N_ADJACENT_LINES).AND.
     '                    .NOT.(((N1.EQ.TRIANGLES(M)).AND.
     '                    (N2.EQ.TRIANGLES(M-1))).OR.
     '                    ((N2.EQ.TRIANGLES(M)).AND.
     '                    (N1.EQ.TRIANGLES(M-1)))))
                          L=L+1
                          M=M-2
                        ENDDO
                        IF (L.LE.N_ADJACENT_LINES) THEN
                          IF (L.LT.N_ADJACENT_LINES) THEN
                            L=3*MAX_TRIANGLES-2*N_ADJACENT_LINES+2
                            TRIANGLES(M)=TRIANGLES(L)
                            TRIANGLES(M-1)=TRIANGLES(L-1)
                          ENDIF
                          N_ADJACENT_LINES=N_ADJACENT_LINES-1
                        ELSE
                          IF (3*N_TRIANGLES+2*(N_ADJACENT_LINES+1).LE.
     '                      3*MAX_TRIANGLES) THEN
C                           add to the list of adjacent vertices
                            L=3*MAX_TRIANGLES-2*N_ADJACENT_LINES
                            TRIANGLES(L)=N2
                            TRIANGLES(L-1)=N1
                            N_ADJACENT_LINES=N_ADJACENT_LINES+1
                          ELSE
                            IERROR=1
                          ENDIF
                        ENDIF
                        N=N+1
                      ENDDO
                      IF (0.EQ.IERROR) THEN
C                       remove triangle
                        IF (J.LT.N_TRIANGLES) THEN
                          L=3*J-2
                          M=3*N_TRIANGLES-2
                          DO N=1,3
                            ITEMP=TRIANGLES(L)
                            TRIANGLES(L)=TRIANGLES(M)
                            TRIANGLES(M)=ITEMP
                            L=L+1
                            M=M+1
                          ENDDO
                          L=4*J-3
                          M=4*N_TRIANGLES-3
                          DO N=1,4
                            RTEMP=RWORKING(L)
                            RWORKING(L)=RWORKING(M)
                            RWORKING(M)=RTEMP
                            L=L+1
                            M=M+1
                          ENDDO
                        ENDIF
                        K=K+1
                        N_TRIANGLES=N_TRIANGLES-1
                      ENDIF
                    ELSE
                      J=J+1
                    ENDIF
                  ENDIF
                ENDDO
                IF (0.EQ.IERROR) THEN
C                 determine new triangles
                  IF (N_TRIANGLES+N_ADJACENT_LINES.LE.MAX_TRIANGLES)
     '              THEN
                    L=3*MAX_TRIANGLES-2*N_ADJACENT_LINES
                    M=3*N_TRIANGLES+1
                    K=1
                    DO WHILE ((0.EQ.IERROR).AND.(K.LE.N_ADJACENT_LINES))
                      TRIANGLES(M)=I/3+1
                      M=M+1
                      L=L+1
                      TRIANGLES(M)=TRIANGLES(L)
                      M=M+1
                      L=L+1
                      TRIANGLES(M)=TRIANGLES(L)
                      M=M+1
                      N_TRIANGLES=N_TRIANGLES+1
                      CALL SPHERE_CALC_CIRCUM(
     '                  VERTICES_XYZ(3*TRIANGLES(3*N_TRIANGLES-2)-2),
     '                  VERTICES_XYZ(3*TRIANGLES(3*N_TRIANGLES-1)-2),
     '                  VERTICES_XYZ(3*TRIANGLES(3*N_TRIANGLES)-2),
     '                  RWORKING(4*N_TRIANGLES-3),IERROR,ERROR,*9999)
                      IF (0.EQ.IERROR) THEN
C                       Calculate radius of triangle
                        CALL SPHERE_CALC_DIST(
     '                    RWORKING(4*N_TRIANGLES-3),VERTICES_XYZ(
     '                    3*TRIANGLES(3*N_TRIANGLES-2)-2),
     '                    RWORKING(4*N_TRIANGLES),IERROR,ERROR,*9999)
                      ENDIF
                      K=K+1
                    ENDDO
                  ELSE
                    IERROR=1
                  ENDIF !N_TRIANGLES
                ENDIF !((0.EQ.IERROR)..)
              ENDIF
              I=I+3
            ENDDO !WHILE((0.EQ.IERROR)...)
          ENDIF
        ELSE
          IERROR=1
        ENDIF !(2.LT.MAX..)
      ELSE
        IF (3.EQ.N_VERTICES) THEN
          IF (2.LE.MAX_TRIANGLES) THEN
C           triangulation covers the sphere
            N_TRIANGLES=2
C           triangle 1
            TRIANGLES(1)=1
            TRIANGLES(2)=2
            TRIANGLES(3)=3
C           triangle 2
            TRIANGLES(4)=1
            TRIANGLES(5)=3
            TRIANGLES(6)=2
          ELSE
            IERROR=1
          ENDIF
        ELSE
          IF (0.LT.N_VERTICES) THEN
            N_TRIANGLES=0
          ELSE
            IERROR=3
          ENDIF
        ENDIF
      ENDIF !(2.LT.NUM...)
c?????debug
      write(*,*) 'finished ',ierror
      if (0.eq.ierror) then
c        do m=1,N_TRIANGLES
c          write(*,*) '  ',m,triangles(3*m-2),triangles(3*m-1),
c     '      triangles(3*m),rworking(4*m-3),rworking(4*m-2),
c     '      rworking(4*m-1),rworking(4*m)
c        enddo
        rtemp=rworking(4)
        do m=2,N_TRIANGLES
          if (rtemp.lt.rworking(4*m)) then
            rtemp=rworking(4*m)
          endif
        enddo
        write(*,*) 'largest radius=',rtemp
      endif

      CALL EXITS('SPHERE_DELAUNAY')
      RETURN
 9999 CALL ERRORS('SPHERE_DELAUNAY',ERROR)
      CALL EXITS('SPHERE_DELAUNAY')
      RETURN 1
      END
