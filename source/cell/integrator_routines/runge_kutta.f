      SUBROUTINE RUNGE_KUTTA(AII,AIO,CONTROL,MODEL,NUM_ODES,NUM_VARS,
     '  SIZES,VARIANT,ARI,ARO,DERIVED,PARAMETERS,PROTOCOL,
     '  T,DT,TIME,YQS,RHSROUTINE,INTEGER_CONTROL,REAL_CONTROL,
     '  INTEGER_WORK,REAL_WORK,ERROR)

C#### Subroutine: RUNGE_KUTTA
C###  Description:
C###    <html>
C###    <p>
C###    This procedure integrates a system of NUM_ODES ordinary
C###    differential equations of the form:<br>
C###      dy(i)/dt=RHSROUTINE(t,y(1),y(2),...,y(NUMBER_EQN)) <br>
C###    from t=T to t=Tout given y(i) at t=T using a 4th order
C###    Runge-Kutta integration scheme, where NUM_VARS is the total
C###    number of variables. </p><p>
C###    This procedure replaces that which was formerly in embedded in
C###    the INTEGRATOR() routine.
C###    </p>
C###    </html>
C###  See-Also: INTEGRATOR

      IMPLICIT NONE

      INCLUDE 'cell_reserved.inc'
      INCLUDE 'integrator_reserved.inc'

      !Parameter list
      INTEGER AII(*),AIO(*),CONTROL(*),MODEL(*),NUM_ODES,NUM_VARS,
     '  SIZES(11),VARIANT
      INTEGER INTEGER_CONTROL(*),INTEGER_WORK(*)
      REAL*8 ARI(*),ARO(*),DERIVED(*),PARAMETERS(*),PROTOCOL(*),TIME(*),
     '  YQS(*),T,DT
      REAL*8 REAL_CONTROL(*),REAL_WORK(*)
      CHARACTER ERROR*(*)
      EXTERNAL RHSROUTINE
      !Local variables
      INTEGER i
      
      IF(INTEGER_CONTROL(SET_DEFAULT_VALUES).EQ.1) THEN
        REAL_CONTROL(ABSOLUTE_ERROR) = 0.0d0 !Unused
        REAL_CONTROL(RELATIVE_ERROR) = 0.0d0 !Unused
        REAL_CONTROL(MAXIMUM_STEP_SIZE) = 0.0d0 !Unused
        INTEGER_CONTROL(ERROR_CODE) = 0
        INTEGER_CONTROL(MAXIMUM_ITERATIONS) = 0 !Unused
        INTEGER_CONTROL(MAXIMUM_ORDER) = 0 !Unused
        INTEGER_CONTROL(ERROR_TYPE) = 0 !Unused
        INTEGER_CONTROL(EXTEND_INTERVAL) = 0 !Unused
        INTEGER_CONTROL(USE_ROUNDING_CONTROL) = 0 !Unused
        INTEGER_CONTROL(INTEGER_WORK_SIZE) = -1
        INTEGER_CONTROL(REAL_WORK_SIZE) = -1
        INTEGER_CONTROL(GET_WORK_SIZES) = 0
      ELSE IF(INTEGER_CONTROL(GET_WORK_SIZES).EQ.1) THEN
        INTEGER_CONTROL(REAL_WORK_SIZE) = 5*NUM_VARS
        INTEGER_CONTROL(INTEGER_WORK_SIZE) = 0
      ELSE
        IF(INTEGER_CONTROL(REAL_WORK_SIZE).GE.(5*NUM_VARS)) THEN
          TIME(TCell) = T
          TIME(DTCell) = DT/2.0d0
          CALL RHSROUTINE(TIME,YQS,REAL_WORK(1+NUM_VARS),CONTROL,MODEL,
     '      SIZES,VARIANT,DERIVED,PARAMETERS,PROTOCOL,AII,AIO,ARI,ARO,
     '      INTEGER_CONTROL(ERROR_CODE))
          IF(INTEGER_CONTROL(ERROR_CODE).NE.0) THEN
            ERROR='>>Error reported from RHSROUTINE'
            INTEGER_CONTROL(ERROR_CODE) = RHS_ERROR
          ELSE
            DO i=1,NUM_ODES
              REAL_WORK(i)=YQS(i)+DT/2.0d0*REAL_WORK(i+NUM_VARS)
            ENDDO !i
            !copy across the non-ODE variables
            DO i=NUM_ODES+1,NUM_VARS
              REAL_WORK(i)=YQS(i)
            ENDDO !i

            TIME(TCell) = T+DT/2.0d0
            TIME(DTCell) = DT/2.0d0
            CALL RHSROUTINE(TIME,REAL_WORK,REAL_WORK(1+(2*NUM_VARS)),
     '        CONTROL,MODEL,SIZES,VARIANT,DERIVED,PARAMETERS,PROTOCOL,
     '        AII,AIO,ARI,ARO,INTEGER_CONTROL(ERROR_CODE))
            
            IF(INTEGER_CONTROL(ERROR_CODE).NE.0) THEN
              ERROR='>>Error reported from RHSROUTINE'
              INTEGER_CONTROL(ERROR_CODE) = RHS_ERROR
            ELSE
              DO i=1,NUM_ODES
                REAL_WORK(i)=YQS(i)+DT/2.0d0*REAL_WORK(i+(2*NUM_VARS))
              ENDDO !i
              !copy across the non-ODE variables
              !DO i=NUM_ODES+1,NUM_VARS
              !  REAL_WORK(i)=YQS(i)
              !ENDDO !i

              TIME(TCell) = T+DT/2.0d0
              TIME(DTCell) = DT/2.0d0
              CALL RHSROUTINE(TIME,REAL_WORK,REAL_WORK(1+(3*NUM_VARS)),
     '          CONTROL,MODEL,SIZES,VARIANT,DERIVED,PARAMETERS,PROTOCOL,
     '          AII,AIO,ARI,ARO,INTEGER_CONTROL(ERROR_CODE))
            
              IF(INTEGER_CONTROL(ERROR_CODE).NE.0) THEN
                ERROR='>>Error reported from RHSROUTINE'
                INTEGER_CONTROL(ERROR_CODE) = RHS_ERROR
              ELSE
                DO i=1,NUM_ODES
                  REAL_WORK(i)=YQS(i)+DT*REAL_WORK(i+(3*NUM_VARS))
                ENDDO !i
                !copy across the non-ODE variables
                !DO i=NUM_ODES+1,NUM_VARS
                !  REAL_WORK(i)=YQS(i)
                !ENDDO !i

                TIME(TCell) = T+DT
                TIME(DTCell) = DT
                CALL RHSROUTINE(TIME,REAL_WORK,REAL_WORK(1+(4
     '            *NUM_VARS)),CONTROL,MODEL,SIZES,VARIANT,DERIVED,
     '            PARAMETERS,PROTOCOL,AII,AIO,ARI,ARO,
     '            INTEGER_CONTROL(ERROR_CODE))
                
                IF(INTEGER_CONTROL(ERROR_CODE).NE.0) THEN
                  ERROR='>>Error reported from RHSROUTINE'
                  INTEGER_CONTROL(ERROR_CODE) = RHS_ERROR
                ELSE
                  !only update the ODE variables
                  DO i=1,NUM_ODES
                    YQS(i)=YQS(i)+DT*(REAL_WORK(i+NUM_VARS)/6.0d0
     '                +REAL_WORK(i+(2*NUM_VARS))/3.0d0+REAL_WORK(i+(3
     '                *NUM_VARS))/3.0d0+REAL_WORK(i+(4*NUM_VARS))/6.0d0)
                  ENDDO !i
                ENDIF !IF(ERR.NE.0)
              ENDIF !IF(ERR.NE.0)
            ENDIF !IF(ERR.NE.0)
          ENDIF !IF(ERR.NE.0)
      
        ELSE
          INTEGER_CONTROL(ERROR_CODE) = INCREASE_REAL_WORK
          ERROR = '>>Error: REAL_WORK_SIZE needs to be >= NUM_VARS'
        ENDIF
      ENDIF !IF(INTEGER_CONTROL(GET_WORK_SIZES).EQ.1) THEN ... ELSE
      
      RETURN
      END
