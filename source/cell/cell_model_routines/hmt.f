      SUBROUTINE HMT_CELL(TIME,Y,DY,CONTROL,MODEL,SIZES,VARIANT,DERIVED,
     '  PARAM,PROTOCOL,AII,AIO,ARI,ARO,ERR_CODE)

C#### Subroutine: HMT_CELL
C###  Description:
C###    Solve isometric twitch using HMT cardiac mechanics model.

      IMPLICIT NONE

      INCLUDE 'cell_hmt.inc'
      INCLUDE 'cell_reserved.inc'

!     Parameter List
      INTEGER SIZES(11)
      INTEGER CONTROL(*),MODEL(*),VARIANT,AII(*),AIO(*),ERR_CODE
      REAL*8 TIME(*),Y(*),PARAM(*),PROTOCOL(*),ARI(*),DY(*),
     '  DERIVED(*),ARO(*)
!     Local variables
      REAL*8 Cb_norm,C50,dPHI1,dPHI2,dPHI3,LENGTH_DIFF,pCa,Q,
     '  TTo,Tm_n,Tm_p50
      REAL*8 ZERO_TOL
      PARAMETER(ZERO_TOL=1.0D-08)
      LOGICAL DEBUG

      ERR_CODE=0

      DEBUG=.FALSE.

C *** DPN 05 July 2000 - need to check that the extension ratio has not
C *** exceeded the physiological limit, and that the proportion of actin
C *** sites does not fall outside the range 0 <= z < 1
C *** DPN 27 September 2000 - Still solve the model even if the values
C     are out of range, simply set them to the limits - this is to allow
C     distributed models to exceed the limits while trying to converge.
      IF ((Y(ExtensionRatio)-PARAM(MaxExtRatio)).GT.ZERO_TOL) THEN
        WRITE(*,
     '    '(''WARNING: Extension ratio above upper limit '',E12.5)')
     '    Y(ExtensionRatio)
        Y(ExtensionRatio) = PARAM(MaxExtRatio)
      ELSEIF ((PARAM(MinExtRatio)-Y(ExtensionRatio)).GT.ZERO_TOL) THEN
        WRITE(*,
     '    '(''WARNING: Extension ratio below lower limit '',E12.5)')
     '    Y(ExtensionRatio)
        Y(ExtensionRatio) = PARAM(MinExtRatio)
      ENDIF
      IF (Y(z).LT.0.0d0) THEN
        WRITE(*,'(''WARNING: Z below lower limit '',E12.5)') Y(z)
        Y(z) = 0.0d0
      ELSEIF (Y(z).GE.1.0d0) THEN
        WRITE(*,'(''WARNING: Z above upper limit '',E12.5)') Y(z)
        Y(z) = 1.0d0
      ENDIF
      LENGTH_DIFF = Y(ExtensionRatio)-Y(ExtensionRatio_prev)
C *** DPN 05 July 2000 - Allow user to solve steady state equations
      IF(CONTROL(STEADY_STATE).EQ.1.OR.LENGTH_DIFF.GT.0.0d0) THEN !Solve steady state equations
        IF(CONTROL(ODE).EQ.1) THEN !evalulating DY for ODEs
C ***     Vm is always used!!
          DY(Vm)=0.0d0
          DY(Cab)=0.0d0
          DY(z)=0.0d0
        ELSE !Non-ODE variables
C ***     For steady state case, all variables except Vm are non-ODE's
          Y(Cab) = Y(Cai)*PARAM(Cab_max)/(Y(Cai)+
     '      (PARAM(Rho1)/PARAM(Rho0))*(1.0d0-1.0d0/PARAM(gamma)))
          Tm_n = PARAM(Tm_n_0)*(1.d0+PARAM(beta1)*
     '      (Y(ExtensionRatio)-1.d0)) 
          Tm_p50 = PARAM(Tm_p50_0)*(1.d0+PARAM(beta2)*
     '      (Y(ExtensionRatio)-1.d0))
          C50 = 10**(3.d0-Tm_p50) !mM
          Y(z) = Y(Cab)**Tm_n/(Y(Cab)**Tm_n+C50**Tm_n)
          Y(IsometricTension) = PARAM(Tref)*(1.0d0+PARAM(beta0)*
     '      (Y(ExtensionRatio)-1.0d0))*Y(z)
          Y(Tension) = Y(IsometricTension)
          Y(ExtensionRatio_prev) = Y(ExtensionRatio)
          Y(phi1) = 0.0d0
          Y(phi2) = 0.0d0
          Y(phi3) = 0.0d0
        ENDIF !ODE
      ELSE ! solve dynamic equations

        IF(CONTROL(ODE).EQ.1) THEN !evalulating DY for ODEs
          
C ***     Vm is always used!!
          DY(Vm)=0.0d0
          
C ***     Troponin Kinetics
          IF (DABS(Y(IsometricTension)).LT.ZERO_TOL) THEN
            TTo = 0.0d0
          ELSE
            TTo = Y(Tension)/(PARAM(gamma)*Y(IsometricTension))
          ENDIF
          DY(Cab)=PARAM(Rho0)*Y(Cai)*(PARAM(Cab_max)-Y(Cab))-
     '      PARAM(Rho1)*(1.0d0-TTo)*Y(Cab)
          
C ***     Tropomyosin Kinetics
C         length dependence for n
          Tm_n   = PARAM(Tm_n_0)*(1.d0+PARAM(beta1)*
     '      (Y(ExtensionRatio)-1.d0)) 
C         length dependence for p50
          Tm_p50 = PARAM(Tm_p50_0)*(1.d0+PARAM(beta2)*
     '      (Y(ExtensionRatio)-1.d0))
          C50 = 10**(3.d0-Tm_p50) !mM

C DPN 05/08/98 - scale dz/dt by normalised [Ca]b
          Cb_norm = Y(Cab)/PARAM(Cab_max)

          DY(z)=PARAM(alfa0)*(((Y(Cab)/C50)*Cb_norm)**Tm_n*
     '      (1.d0-Y(z)) - Y(z))
          
        ELSE !evaluating new values for non-ODE state variables

C ***     Intracellular Ca++
C         Only calculate Cai if the user wants to, i.e. they might be 
C         setting the value via a time variable.
          IF(MODEL(CALCULATE_Cai).NE.0) 
     '      Y(Cai)=PARAM(Ca_min)+PARAM(Ca_max)*
     '      TIME(TCell)/PARAM(Ca_tau)*DEXP(1.d0-TIME(TCell)/
     '      PARAM(Ca_tau))

C ***     Tension-length-pCa - Isometric Tension
          Y(IsometricTension) = PARAM(Tref)*(1.d0+PARAM(beta0)*
     '      (Y(ExtensionRatio)-1.d0))*Y(z)
          
C ***     X-bridge kinetics - Active tension
C         Fading memory model...

C         1st Fading Memory term
          dPHI1=0.5d0*LENGTH_DIFF*(1.d0+DEXP(-PARAM(alpha1)*
     '      TIME(DTCell)))
          Y(phi1)=DEXP(-PARAM(alpha1)*TIME(DTCell))*Y(phi1)+dPHI1
          
C         2nd Fading Memory term
          dPHI2=0.5d0*LENGTH_DIFF*(1.d0+DEXP(-PARAM(alpha2)*
     '      TIME(DTCell)))
          Y(phi2)=DEXP(-PARAM(alpha2)*TIME(DTCell))*Y(phi2)+dPHI2
          
C         3rd Fading Memory term
          dPHI3=0.5d0*LENGTH_DIFF*(1.d0+DEXP(-PARAM(alpha3)*
     '      TIME(DTCell)))
          Y(phi3)=DEXP(-PARAM(alpha3)*TIME(DTCell))*Y(phi3)+dPHI3
          
          Q = PARAM(A1)*Y(phi1)+PARAM(A2)*Y(phi2)+PARAM(A3)*Y(phi3)

C *** DPN 20 July 2000 - Need to fix HMT for lengthening experiments
          IF(Q.LT.0.00d0) THEN
            Y(Tension)=Y(IsometricTension)*(1.d0+PARAM(a)*Q)/(1.d0-Q)
          ELSE
            Y(Tension) = Y(IsometricTension)
          ENDIF
          
C         Update previous extension ratio
          Y(ExtensionRatio_prev) = Y(ExtensionRatio)

        ENDIF
      ENDIF !steady state/dynamic

      IF(DEBUG) WRITE(*,'('' t='',F5.3,'' Cai='',E12.3,''(uM)'''
     '  //''' pCa='',E12.3,'
     '  //' '' Cb='',E12.3,''(uM)'''
     '  //'  '' z='',E12.4)') 
     '  TIME(TCell),Y(Cai),pCa,Y(Cab),Y(z)
      
      RETURN
      END

