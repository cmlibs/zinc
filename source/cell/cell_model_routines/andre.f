      SUBROUTINE ANDRES_CELL_MODEL(TIME,Y,DY,CONTROL,MODEL,SIZES,
     '  VARIANT,DERIVED,PARAM,PROTOCOL,AII,AIO,ARI,ARO,ERR_CODE)

C#### Subroutine: ANDRES_CELL_MODEL
C###  Description:
C###    Solve stuff

      IMPLICIT NONE

!     Parameter List
      INTEGER SIZES(11)
      INTEGER CONTROL(*),MODEL(*),VARIANT,AII(*),AIO(*),ERR_CODE
      REAL*8 TIME(*),Y(*),PARAM(*),PROTOCOL(*),ARI(*),DY(*),
     '  DERIVED(*),ARO(*)
!     Local variables
      INTEGER Vm,Cao,Cai,Tension,ICa,ODE,TCell,DTCell
      ! Time
      PARAMETER(TCell=1)
      PARAMETER(DTCell=2)
      ! State
      PARAMETER(Vm=1)
      PARAMETER(Tension=2)
      ! Derived
      PARAMETER(ICa=1)
      ! Control
      PARAMETER(ODE=1)
      ! Material
      PARAMETER(Cao=1)
      PARAMETER(Cai=2)
      
      ERR_CODE=0

      IF(CONTROL(ODE).EQ.1) THEN !evalulating DY for ODEs
          
        DY(Vm)=PARAM(Cao)*2.0d0*TIME(TCell)

        DERIVED(ICa)=PARAM(Cai)*TIME(TCell)**2.0d0
        
        DY(Tension)=0.0d0
          
      ELSE !evaluating new values for non-ODE state variables

        Y(Tension)=PARAM(Cai)*TIME(TCell)

      ENDIF
      
      RETURN
      END
