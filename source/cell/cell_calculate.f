#define HH_TIMESCALE PARAMETERS( 1)
#define HH_TSTART    PARAMETERS( 2)
#define HH_TEND      PARAMETERS( 3)
#define HH_DT        PARAMETERS( 4)
#define HH_TABT      PARAMETERS( 5)
#define HH_TPS       PARAMETERS( 6)
#define HH_TP        PARAMETERS( 7)
#define HH_g_Na      PARAMETERS( 8)
#define HH_g_K       PARAMETERS( 9)
#define HH_g_L       PARAMETERS(10)
#define HH_ic_Na     PARAMETERS(11)
#define HH_ec_Na     PARAMETERS(12)
#define HH_ic_K      PARAMETERS(13)
#define HH_ec_K      PARAMETERS(14)
#define HH_T         PARAMETERS(15)
#define HH_VL        PARAMETERS(16)
#define HH_Cap       PARAMETERS(17)
#define HH_ISTIM     PARAMETERS(18)
#define HH_TPS2      PARAMETERS(19)
#define HH_ISTIM2    PARAMETERS(20)
#define HH_VNa       ARI(1)
#define HH_VK        ARI(2)
#define HH_Vrest     ARI(3)
#define HH_Stimulus  ARO( 1)
#define HH_INa       ARO( 2)
#define HH_IK        ARO( 3)
#define HH_Ileak     ARO( 4)
#define HH_alpha_h   ARO( 5)
#define HH_beta_h    ARO( 6)
#define HH_alpha_m   ARO( 7)
#define HH_beta_m    ARO( 8)
#define HH_alpha_n   ARO( 9)
#define HH_beta_n    ARO(10)
#define HH_Stim_tv   CONTROL(1)
#define HH_Stim_id   18
#define HH_Stim_tv_current_position AIO(1)
#define LR_Number_of_stimulii AIO(2)
#define LR_TIMESCALE PARAMETERS( 1)
#define LR_TSTART    PARAMETERS( 2)
#define LR_TEND      PARAMETERS( 3)
#define LR_DT        PARAMETERS( 4)
#define LR_TABT      PARAMETERS( 5)
#define LR_TPS       PARAMETERS( 6)
#define LR_TP        PARAMETERS( 7)
#define LR_GGCab    PARAMETERS(8) 
#define LR_KmCa_L   PARAMETERS(9)
#define LR_PCa   PARAMETERS(10)
#define LR_gCao   PARAMETERS(11)
#define LR_kNaCa   PARAMETERS(12)
#define LR_KmNa   PARAMETERS(13)
#define LR_KmCa_NaCa   PARAMETERS(14)
#define LR_eta   PARAMETERS(15)
#define LR_ksat   PARAMETERS(16)
#define LR_KmpCa   PARAMETERS(17)
#define LR_IIpCa   PARAMETERS(18)
#define LR_KmnsCa   PARAMETERS(19)
#define LR_PnsCa   PARAMETERS(20)
#define LR_Cm   PARAMETERS(21)
#define LR_Acap   PARAMETERS(22)
#define LR_Temp   PARAMETERS(23)
#define LR_VolMyo   PARAMETERS(24)
#define LR_CSQNCSQN   PARAMETERS(25)
#define LR_KmCSQN   PARAMETERS(26)
#define LR_Kmrel   PARAMETERS(27)
#define LR_Tau_on   PARAMETERS(28)
#define LR_Tau_off   PARAMETERS(29)
#define LR_Caith   PARAMETERS(30)
#define LR_GGrel_   PARAMETERS(31)
#define LR_VolJSR   PARAMETERS(32)
#define LR_Tau_tr   PARAMETERS(33)
#define LR_Kmup   PARAMETERS(34)
#define LR_CaNSRCaNSR   PARAMETERS(35)
#define LR_IIup   PARAMETERS(36)
#define LR_VolNSR   PARAMETERS(37)
#define LR_gCai   PARAMETERS(38)
#define LR_TRPNTRPN   PARAMETERS(39)
#define LR_KmTRPN   PARAMETERS(40)
#define LR_CMDNCMDN   PARAMETERS(41)
#define LR_KmCMDN   PARAMETERS(42) 
#define LR_GGKp   PARAMETERS(43)
#define LR_PK   PARAMETERS(44)
#define LR_gNai   PARAMETERS(45)
#define LR_gKi   PARAMETERS(46)
#define LR_gNao   PARAMETERS(47)
#define LR_gKo   PARAMETERS(48)
#define LR_PNaK   PARAMETERS(49)
#define LR_IINaK   PARAMETERS(50)
#define LR_KmNai   PARAMETERS(51)
#define LR_KmKo   PARAMETERS(52)
#define LR_GGNab   PARAMETERS(53)
#define LR_GGNa   PARAMETERS(54) 
#define LR_PNa   PARAMETERS(55)
#define LR_Istim   PARAMETERS(56)
#define LR_Ko   PARAMETERS(57)
#define LR_Nao   PARAMETERS(58)
#define LR_tps   PARAMETERS(58)
#define LR_numStim   PARAMETERS(59)
#define LR_dtp   PARAMETERS(60)
#define LR_Farad   96.5d3 /*C/mol*/
#define LR_Stimulus  ARO( 1)
#define LR_INa       ARO( 2)
#define LR_ICaL      ARO( 3)
#define LR_IK        ARO( 4)
#define LR_IK1       ARO( 5)
#define LR_IKp       ARO( 6)
#define LR_INaCa     ARO( 7)
#define LR_INaK      ARO( 8)
#define LR_InsCa     ARO( 9)
#define LR_IpCa      ARO(10)
#define LR_ICab      ARO(11)
#define LR_INab      ARO(12)
#define LR_Iv        ARO(13)
#define LR_Irel      ARO(14)
#define LR_Iup       ARO(15)
#define LR_Ileak     ARO(16)
#define LR_Itr       ARO(17)
#define LR_ICaLCa    ARO(18)
#define LR_ICaLK     ARO(19)
#define LR_ICaLNa    ARO(20)
#define LR_I_Na      ARO(21)
#define LR_I_Ca      ARO(22)
#define LR_I_K       ARO(23)
#define LR_F_JSR     ARO(24)
#define LR_F_NSR     ARO(25)
      SUBROUTINE ADAMS(SUB_NAME,NT_EQN,TBEG,HMAX,ALPH,H,HOLD,
     '  T,THIRD,H202,H303,Y,F,D1,DDIF,JFLAG,NFE,NSTEP,NBUMP,IFLAG)

C#### Subroutine: ADAMS
C###  Description:
C###    This procedure integrates the solution y from t to t+h using a 
C###    3rd order variable stepsize Adams p2ec3e formula. 
C###    On first call Adams also sets coefficients and does a pe1ce2 
C###    step. After each step the estimated local error is tested. 
C###    If the error is too large, the step is rejected and repeated 
C###    with smaller h. The variables are defined as they are used.

      IMPLICIT NONE
c      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      INTEGER IFLAG,JFLAG,NBUMP,NFE,NSTEP,NT_EQN
      REAL*8 ALPH,BET,D1(*),DDIF(*),F(*),H,H202,H303,HMAX,HOLD,T,TBEG,
     '  THIRD,Y(*)
      CHARACTER SUB_NAME*(*)
!     Local Variables
      INTEGER J,M,NREDO!,I
      REAL*8 D2(99),EPS,EPSP1,ERR,ERRD,ERREPS,ERRMAX,ERTEST,FP(99),
     '  H1,HDEMX,HMIN,P(99),R,RMAX,TTST,Z,membrane_current
      LOGICAL DISCONT

c      VOLATILE ERRMAX,HMIN,ALPH,BET

!      write(*,'('' >>>call adams'')')
      DISCONT=.FALSE.
 1    IF(IFLAG.EQ.0) THEN
        IF(DISCONT) GOTO 11

C ***   Compute machine epsilon.
        EPS=1.0d0
        EPSP1=2.0d0
        DO WHILE(EPSP1.GT.1.0d0)
          EPS=0.5d0*EPS
          EPSP1=EPS+1.0d0
        ENDDO
        EPS=2.0d0*EPS
        
!!!        WRITE(*,*) 'eps',EPS,EPSP1

C ***   hmin is smallest allowed step size and starting stepsize.
        HMIN=100.0d0*EPS

C ***   errmax is largest acceptable relative local error.
        ERRMAX=0.005d0
!!!        WRITE(*,*) 'ERRMAX = ',ERRMAX
C ***   erreps is inserted into denom of error to prevent / by zero.
        ERREPS=0.001d0*ERRMAX
!!!        WRITE(*,*) 'ERREPS = ',ERREPS

C ***   After each step the value of next stepsize is estimated. This
C ***   value is multiplied by alph to be on safe side. If a step is
C ***   rejected, h is multiplied by bet before trying again.
        ALPH=0.5d0
        BET=0.7d0
        THIRD=1.0d0/3.0d0
        NBUMP=0
        T=TBEG
!!!        WRITE(*,*) ALPH,BET,THIRD,NBUMP,T

C Resetting initial H to avoid underflow with optimisation
C 11     H=HMIN
 11     H=0.0001d0
        JFLAG=1

C ***   First step uses euler predictor and trapezoidal corrector.
        H202=0.0d0
        H303=0.5d0*H*H
        HOLD=1.0d0-H
!!!        WRITE(*,*) 'h202,h303,hold',h202,h303,hold

        IF(SUB_NAME(1:2).EQ.'HH') THEN       !Hodgkin-Huxley eqns
c          CALL HH(T,Y,F)
        ELSE IF(SUB_NAME(1:2).EQ.'BR') THEN  !Beeler-Reuter eqns
!         CALL BR(T,Y,F)
        ELSE IF(SUB_NAME(1:3).EQ.'RBR') THEN !Reduced Beeler-Reuter eqns
!         CALL RBR(T,Y,F)
        ELSE IF(SUB_NAME(1:4).EQ.'DN') THEN  !diFrancesco-Noble eqns
          CALL DN(T,Y,F)
        ELSE IF(SUB_NAME(1:2).EQ.'LR') THEN  !Luo-Rudy eqns
          CALL LR(T,Y,F)
!         IF(DOP) THEN
!            WRITE(99,'('' T='',E11.3,'' IFLAG='',I3)') T,IFLAG
!            WRITE(99,'('' Y( 1..10):'',10E11.3)') (Y(I),I= 1,10)
!            WRITE(99,'('' Y(11..20):'',10E11.3)') (Y(I),I=11,20)
!            WRITE(99,'('' Y(21..30):'',10E11.3)') (Y(I),I=21,30)
c            WRITE(99,'('' T='',E11.3,'' IFLAG='',I3)') T,IFLAG
c            WRITE(99,'('' Y( 1.. 7):'',10E11.3)') (Y(I),I= 1, 7)
c            WRITE(99,'('' Y( 8..14):'',10E11.3)') (Y(I),I= 8,14)
!         ENDIF
        ELSE IF(SUB_NAME(1:3).EQ.'JRW') THEN  !Jafri-Rice-Winslow eqns
          CALL JRW(T,Y,F)
        ELSE IF(SUB_NAME(1:2).EQ.'DM') THEN  !Distribution moment model
          CALL FCN_DM(T,Y,F)
        ELSE IF(SUB_NAME(1:3).EQ.'N98') THEN  !Noble98 model
          CALL NOBLE98(T,Y,F)
        ELSE IF(SUB_NAME(1:3).EQ.'ATR') THEN  !atrial model
          CALL ATR(T,Y,F)
        ENDIF
        NFE=NFE+1
        DO J=1,NT_EQN
          D1(J)=0.0d0
!!!          WRITE(*,*) 'y,f',y(j),f(j)
        ENDDO
      ENDIF

!!!      WRITE(*,*) 'h',H
!!!      WRITE(*,*) 'nfe',nfe
!!!      WRITE(*,*) 'jflag',jflag
!!!      WRITE(*,*) 'hmin',hmin

      IF(IFLAG.LT.2) IFLAG=IFLAG-1
      DO WHILE(IFLAG.LT.1)
        IFLAG=IFLAG+1

C ***   rmax is maximum relative stepsize increase 10 usually,
C ***   but set to 2 after rejected step

        EPS=1.0d0
        EPSP1=2.0d0
        DO WHILE(EPSP1.GT.1.0d0)
          EPS=0.5d0*EPS
          EPSP1=EPS+1.0d0
        ENDDO
        EPS=2.0d0*EPS
        ALPH=0.5d0
        BET=0.7d0
        HMIN=100.0d0*EPS
        RMAX=10.0d0
        IF(JFLAG.EQ.2) RMAX=2.0d0
        IF(IFLAG.EQ.0) RMAX=1.0d5
        NREDO=0
        ERRMAX=0.005d0
        ERREPS=0.001d0*ERRMAX
        JFLAG=1
        M=0

!!!        WRITE(*,*) 'IFLAG,RMAX,NREDO,JFLAG,M',IFLAG,RMAX,NREDO,JFLAG,M

        DO WHILE((IFLAG.LT.2).AND.(M.LT.2))

C ***     Compute predicted values and store in array p
          DO J=1,NT_EQN
            P(J)=Y(J)+H*F(J)+H202*D1(J)
          ENDDO

          M=0
          TTST=T+H
          ERR=0.0d0

!!!          WRITE(*,*) 'ERR,TTST,M',ERR,TTST,M

          DO WHILE((ERR.LT.1.0d0).AND.(M.LT.2))

C ***       Compute derivatives using values in array p
            IF(DABS(P(1)).GT.200.0d0) IFLAG=5
            IF(SUB_NAME(1:2).EQ.'HH') THEN       !Hodgkin-Huxley eqns
c              CALL HH(TTST,P,FP)
            ELSE IF(SUB_NAME(1:2).EQ.'BR') THEN  !Beeler-Reuter eqns
!             CALL BR(TTST,P,FP)
            ELSE IF(SUB_NAME(1:3).EQ.'RBR') THEN !Reduced Beeler-Reuter eqns
!             CALL RBR(TTST,P,FP)
            ELSE IF(SUB_NAME(1:4).EQ.'JRWP') THEN  !JRW (Princeton)
              CALL FCN_JRWP(1,2.0d0,2,P(1),P,FP,membrane_current)       
            ELSE IF(SUB_NAME(1:2).EQ.'LR') THEN  !Luo-Rudy eqns
              CALL LR(TTST,P,FP)
!              WRITE(*,'('' T='',E11.3,'' IFLAG='',I3)') T,IFLAG
!              WRITE(*,'('' Y( 1..10):'',10E11.3)') (Y(I),I= 1,10)
!              WRITE(*,'('' Y(11..20):'',10E11.3)') (Y(I),I=11,20)
!              WRITE(*,'('' Y(21..30):'',10E11.3)') (Y(I),I=21,30)
c              WRITE(99,'('' T='',E11.3,'' IFLAG='',I3)') T,IFLAG
c              WRITE(99,'('' Y( 1.. 7):'',10E11.3)') (Y(I),I= 1, 7)
c              WRITE(99,'('' Y( 8..14):'',10E11.3)') (Y(I),I= 8,14)
            ELSE IF(SUB_NAME(1:3).EQ.'JRW') THEN  !JRW eqns
              CALL JRW(TTST,P,FP)
            ELSE IF(SUB_NAME(1:2).EQ.'DM') THEN  !Distribution moment
              CALL FCN_DM(TTST,P,FP)
            ELSE IF(SUB_NAME(1:3).EQ.'N98') THEN !Noble98 model
              CALL NOBLE98(TTST,P,FP)
            ELSE IF(SUB_NAME(1:3).EQ.'ATR') THEN !atrial model
              CALL ATR(TTST,P,FP)
            ENDIF
            NFE=NFE+1

C ***       Compute first and second divided differences
            DO J=1,NT_EQN
              D2(J)=(FP(J)-F(J))/H
              DDIF(J)=(D2(J)-D1(J))/(H+HOLD)
!!!              WRITE(*,*) 'd2,dif',d2(j),ddif(j)
            ENDDO
!!!            WRITE(*,*) 'h,hold',H,HOLD
            IF(M.EQ.0) THEN
              H1=H303+HOLD*H202

C ***         Compute corrected values & store pending error test.
              DO J=1,NT_EQN
                P(J)=P(J)+H1*DDIF(J)
              ENDDO

C ***         Test estimated local error using l(infinity) norm.
              ERR=0.0d0
              HDEMX=0.5d0*H303/ERRMAX
              IF(IFLAG.EQ.0) HDEMX=0.5d0*HDEMX
!!!              WRITE(*,*) 'hdemx,h303,errmax',hdemx,h303,errmax
              DO J=1,NT_EQN
                ERRD=DABS(P(J))
                IF(ERRD.LT.ERREPS) ERRD=ERREPS
                ERTEST=HDEMX*DABS(DDIF(J))/ERRD
                IF(ERR.LT.ERTEST) ERR=ERTEST
!!!                WRITE(*,*) 'ERRD,ERREPS,ERTEST,DDIF,ERR'
!!!                WRITE(*,*) ERRD,ERREPS,ERTEST,DDIF(j),ERR
              ENDDO
            ENDIF
            M=M+1
          ENDDO

C ***     Error test fails. Reduce h and try again if h .GT. hmin.
C ***     If 6 straight reductions occur, assume discontinuity and start over.
          IF(M.LT.2) THEN
            JFLAG=2
            NREDO=NREDO+1
            IF(NREDO.GE.6) THEN
              IFLAG=0
              DISCONT=.TRUE.
!!!              WRITE(*,*) 'going to 1'
              GOTO 1
            ENDIF
            H=BET*H
            H202=0.5d0*H*H
            H303=THIRD*H*H*H
            IF(H.LE.HMIN) IFLAG=2
!!!            WRITE(*,*) 'H,h202,h303',h,h202,h303
!!!            WRITE(*,*) 'hmin,bet,iflag,jflag,nredo',hmin,bet,iflag,
!!!     '        jflag,nredo
          ENDIF
        ENDDO

        IF(IFLAG.LT.2) THEN
C ***     Step accepted. Update y array and prepare for next step.
          T=TTST
          NSTEP=NSTEP+1
          DO J=1,NT_EQN
            D1(J)=D2(J)
            F(J)=FP(J)
            Y(J)=P(J)
          ENDDO
          IF(SUB_NAME(1:2).EQ.'DN') THEN  !diFrancesco-Noble eqns
!           IF(DOP) THEN
!              WRITE(*,'('' T='',E11.3,'' IFLAG='',I3)') T,IFLAG
!              WRITE(*,'('' Y( 1..10):'',10E11.3)')
!     '    (Y(I),I= 1,10)
!              WRITE(*,'('' Y(11..20):'',10E11.3)')
!     '    (Y(I),I=11,20)
!              WRITE(*,'('' Y(21..30):'',10E11.3)')
!     '    (Y(I),I=21,30)
!           ENDIF
          ENDIF

C ***     Compute stepsize for next step.
          IF(ERR.GT.0) THEN
            Z=THIRD
            IF(IFLAG.EQ.0) Z=0.5d0
!!!            WRITE(*,*) 'ERR',ERR
            R=(ALPH/ERR)**Z
            IF(R.GT.RMAX) R=RMAX
          ELSE
            R=RMAX
          ENDIF
          HOLD=H
          H=R*H
          IF(H.LT.HMIN) H=HMIN
          IF(H.GE.HMAX) THEN
            H=HMAX
C ***       If h bumps against hmax too often hmax is increased.
            NBUMP=NBUMP+1
            IF(NBUMP.GE.20) THEN
              HMAX=2.0d0*HMAX
              NBUMP=0
            ENDIF
          ENDIF
          H202=0.5d0*H*H
          H303=THIRD*H*H*H
        ENDIF
      ENDDO

      RETURN
      END


      SUBROUTINE ADAMS_MOULTON(AII,AIO,CONTROL,ERROR_TYPE,IFAIL,IWORK,
     '  L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_IWORK,L_MODEL,
     '  L_PARAMETERS,L_WORK,MAX_ITERS,MAX_ORDER,MODEL,NUMBER_EQN,
     '  ABS_ERR,ARI,ARO,DY,MAX_STEP,PARAMETERS,REL_ERR,T,TOUT,WORK,Y,
     '  EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL,FUNC,ERROR)

C#### Subroutine: ADAMS_MOULTON
C###  Description:
C###    This procedure integrates a system of NUMBER_EQN ordinary
C###    differential equations of the form 
C###      dy(i)/dt=FUNC(t,y(1),y(2),...,y(NUMBER_EQN)) 
C###    from t=T to t=TOUT given y(i) at t=T using an Adams-Moulton
C###    integration scheme. The integrator used is adapted from the
C###    Adams-Moulton integrator of L.F. Shampine and M.K. Gordon 
C###    and is detailed in their book "Computer Solution of Ordinary
C###    Differential Equations: The Initial Value Problem".
C###    The exact form of the rhs function is
C###      CALL FUNC(T,Y,DY,AII,AIO,CONTROL,L_AII,L_AIO,L_ARI,L_ARO,
C###        L_CONTROL,L_MODEL,L_PARAMETERS,MODEL,NUMBER_EQN,
C###        ARI,ARO,PARAMETERS,ERR)
C###    where CONTROL, MODEL, ARI, ARO are integer vectors of sizes
C###    L_CONTROL, L_MODEL, L_ARI and L_ARO respectively and PARAMETERS,
C###    Y, DY, ARI, ARO are double precision vectors of sizes 
C###    L_PARAMETERS, NUMBER_EQN, NUMBER_EQN, L_ARI and L_ARO 
C###    respectively.
C###    The integrator uses a modified divided difference form of the
C###    Adams PECE formulas and local extropolation. It iterates on 
C###    the solution (up to MAX_ITERS) and adjusts the order (up to 
C###    MAX_ORDER and no more than 12) and step size (up to MAX_STEP)
C###    to control the local error per unit step in a generalised 
C###    sense. 
C###    The error control is based on the L2 Norm of the weighted 
C###    local error vector. The weighting is controlled by ERROR_TYPE.
C###    If ERROR_TYPE=1 then no weighting is used; if ERROR_TYPE=2 
C###    then the error vector components are weighted by the most 
C###    recent component of the solution vector; if ERROR_TYPE=3 then
C###    the error vector components are weighted by the most recent 
C###    component of the residual (derivative) vector; if ERROR_TYPE=4
C###    then a mixed relative weighting is used. This weighting is 
C###    calculated from (REL_ERR*y(i)+ABS_ERR)/MAX(REL_ERR,ABS_ERR).
C###    For reasons of efficiency the integrator integrates beyond
C###    TOUT internally (though never beyond T+10*(TOUT-T)) and 
C###    interpolates the solution at TOUT. If it is not possible to
C###    integrate beyond TOUT then EXTEND_INTERVAL should be set to
C###    .FALSE.
C###    The integrator can perform propagative rounding error control
C###    by setting USE_ROUND_CONTROL to .TRUE.
C###    The integrator needs workspace allocated as follows:
C###      IWORK(L_IWORK) where L_IWORK=7 and
C###      WORK(L_WORK) where L_WORK=7+7*MAX_ORDER+
C###        NUMBER_EQN*(MAX_ORDER+6) in general, and 
C###        L_WORK=7+7*MAX_ORDER+NUMBER_EQN*(MAX_ORDER+8) if rounding
C###        control is required.
C###    The workspace needs to be mainted between calls. Interesting
C###    values contain within the workspace are: 
C###      IWORK(1) - the order, K, of the Adams polynomial to be used 
C###        with the next call,
C###      IWORK(2) - the order of the Adams polynomial used in the 
C###        previous (current) step,
C###      WORK(1) - the step size, H, to be used with the next call,
C###      WORK(2) - the step size used in the previous (current) step.
C###      WORK(3) - the finish time of the last step taken.
C###      WORK(4) - the finish time of the last step output.
C###    On entry the following parameters are required:
C###      NUMBER_EQN - the number of equations to be integrated,
C###      Y - vector of initial conditions,
C###      T - starting point of the integration,
C###      TOUT - point at which the solution is desired,
C###      ABS_ERR, REL_ERR - absolute and relative local error 
C###        tolerances,
C###      ERROR_TYPE - the type of error test to be used,
C###      IFAIL - error code indicator. Must be set to 1 for start-up,
C###      MAX_ITERS - the maximum number of Adams iterations (steps)
C###        allowed to reach TOUT,
C###      MAX_ORDER - the maximum order allowed for the Adams 
C###        polynomial,
C###      MAX_STEP - the maximum step size allowed to reach TOUT,
C###      EXTEND_INTERVAL - set to .TRUE. if it is possible to 
C###        integrate beyond TOUT.
C###      USE_ROUND_CTRL - set to .TRUE. if roudning control is to be
C###        used.
C###    On output the following parameters are set:
C###      Y - vector of solutions at TOUT,
C###      DY - vector of the derivates at TOUT,
C###      T - last point reach in the integration. Normal return has
C###        T=TOUT,
C###      ABS_ERR, REL_ERR - normal return has the tolerances unchanged.
C###        If IFAIL=3 then the tolerances are set to increased values.
C###      STIFF_EQNS - set to .TRUE. if the equations appear to be
C###        stiff,
C###      IFAIL - error flag. If 
C###        IFAIL=2 - normal return, integration reached TOUT,
C###        IFAIL=3 - integration did not reach TOUT because the error
C###          are too small. ABS_ERR and REL_ERR have been increased
C###          to appropriate levels.
C###        IFAIL=4 - integration did not reach TOUT because more than
C###          MAX_ITERS steps were needed,
C###        IFAIL=5 - integration did not reach TOUT because equations
C###          appear to be stiff,
C###        IFAIL=6 - invalid input parameters,
C###        IFAIL=7 - error returned from FUNC,
C###        IFAIL=8 - if ERROR_TYPE is 2 or 3 then one of error weights
C###          is zero,
C###        IFAIL=9 - WORK array is too small,
C###        IFAIL=10 - IWORK array is too small.

      IMPLICIT NONE
      
!     Parameter list
      INTEGER L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_IWORK,L_MODEL,
     '  L_PARAMETERS,L_WORK
      INTEGER AII(L_AII),AIO(L_AIO),CONTROL(L_CONTROL),ERROR_TYPE,
     '  IFAIL,IWORK(L_IWORK),MAX_ITERS,MAX_ORDER,MODEL(L_MODEL),
     '  NUMBER_EQN
      REAL*8 ABS_ERR,ARI(L_ARI),ARO(L_ARO),DY(NUMBER_EQN),
     '  MAX_STEP,PARAMETERS(L_PARAMETERS),REL_ERR,T,TOUT,WORK(L_WORK),
     '  Y(NUMBER_EQN)
      LOGICAL EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL
      EXTERNAL FUNC
      CHARACTER ERROR*(*)
!     Local Variables
      INTEGER MAX_RND
      INTEGER ISNOLD_IDX,K_IDX,KOLD_IDX,NUMBER_STEPS_IDX,
     '  PHASE1_IDX,REQUIRED_IDX,ROUND_CTRL_IDX,START_IDX
      INTEGER H_IDX,HOLD_IDX,TT_IDX,TOLD_IDX,ALPHA_IDX,BETA_IDX,
     '  DELSGN_IDX,DY_IDX,ERROR_WEIGHT_IDX,G_IDX,PREDICTION_IDX,
     '  PSI_IDX,SIGMA_IDX,V_IDX,W_IDX,YY_IDX,PHI_IDX
      LOGICAL ROUND_CTRL,PHASE1,START

      DATA ISNOLD_IDX,K_IDX,KOLD_IDX,NUMBER_STEPS_IDX,PHASE1_IDX,
     '  ROUND_CTRL_IDX,START_IDX / 3,1,2,4,5,6,7 /
      DATA H_IDX,HOLD_IDX,TT_IDX,TOLD_IDX,DELSGN_IDX,
     '  ALPHA_IDX / 1,2,3,4,5,6 /


      IF(L_IWORK.GE.7) THEN
        REQUIRED_IDX=7+7*MAX_ORDER+NUMBER_EQN*(MAX_ORDER+6)
        IF(USE_ROUND_CTRL) REQUIRED_IDX=REQUIRED_IDX+2*NUMBER_EQN
        IF(L_WORK.GE.REQUIRED_IDX) THEN
          BETA_IDX=ALPHA_IDX+MAX_ORDER
          DY_IDX=BETA_IDX+MAX_ORDER
          ERROR_WEIGHT_IDX=DY_IDX+NUMBER_EQN
          G_IDX=ERROR_WEIGHT_IDX+NUMBER_EQN
          PSI_IDX=G_IDX+MAX_ORDER+1
          PREDICTION_IDX=PSI_IDX+MAX_ORDER
          SIGMA_IDX=PREDICTION_IDX+NUMBER_EQN
          V_IDX=SIGMA_IDX+MAX_ORDER+1
          W_IDX=V_IDX+MAX_ORDER
          YY_IDX=W_IDX+MAX_ORDER 
          PHI_IDX=YY_IDX+NUMBER_EQN
          
          IF(USE_ROUND_CTRL) THEN
            MAX_RND=2
          ELSE
            MAX_RND=0
          ENDIF
          
          IF(ABS(IFAIL).NE.1) THEN
            START=IWORK(START_IDX).NE.0
            PHASE1=IWORK(PHASE1_IDX).NE.0
            IF(USE_ROUND_CTRL) THEN
              ROUND_CTRL=IWORK(ROUND_CTRL_IDX).NE.0
            ELSE
              ROUND_CTRL=.FALSE.
            ENDIF
          ENDIF
          
          CALL AM_DE(AII,AIO,CONTROL,ERROR_TYPE,IFAIL,
     +      IWORK(ISNOLD_IDX),IWORK(K_IDX),IWORK(KOLD_IDX),
     +      L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,
     +      L_PARAMETERS,MAX_ITERS,MAX_ORDER,MAX_RND,MODEL,
     +      NUMBER_EQN,IWORK(NUMBER_STEPS_IDX),ABS_ERR,WORK(ALPHA_IDX),
     +      ARI,ARO,WORK(BETA_IDX),WORK(DELSGN_IDX),WORK(DY_IDX),DY,
     +      WORK(ERROR_WEIGHT_IDX),WORK(G_IDX),WORK(H_IDX),
     +      WORK(HOLD_IDX),MAX_STEP,PARAMETERS,WORK(PHI_IDX),
     +      WORK(PREDICTION_IDX),WORK(PSI_IDX),REL_ERR,
     +      WORK(SIGMA_IDX),T,WORK(TT_IDX),WORK(TOLD_IDX),
     +      TOUT,WORK(V_IDX),WORK(W_IDX),Y,WORK(YY_IDX),
     +      EXTEND_INTERVAL,PHASE1,ROUND_CTRL,START,STIFF_EQNS,FUNC)
          
          IF(START) THEN
            IWORK(START_IDX)=1
          ELSE
            IWORK(START_IDX)=0
          ENDIF
          IF(PHASE1) THEN
            IWORK(PHASE1_IDX)=1
          ELSE
            IWORK(PHASE1_IDX)=0
          ENDIF
          IF(USE_ROUND_CTRL) THEN
            IF(ROUND_CTRL) THEN
              IWORK(ROUND_CTRL_IDX)=1
            ELSE
              IWORK(ROUND_CTRL_IDX)=0
            ENDIF
          ENDIF
          
          IF(ABS(IFAIL).EQ.3) THEN
            ERROR='>>Integration did not reach TOUT, Tolerances too '
     '        //'small'
          ELSE IF(ABS(IFAIL).EQ.4) THEN
            ERROR='>>Integration did not reach TOUT, Maximum '
     '        //'iterations exceeded'
          ELSE IF(ABS(IFAIL).EQ.5) THEN
            ERROR='>>Integration did not reach TOUT, Equations are '
     '        //'too stiff'
          ELSE IF(ABS(IFAIL).EQ.6) THEN
            ERROR='>>Input parameters invalid'
          ENDIF
        ELSE
          WRITE(ERROR,'(''>>WORK array is too small, increase L_WORK '
     '      //'to '',I5)') REQUIRED_IDX
          IFAIL=9
        ENDIF
      ELSE
        ERROR='>>IWORK array is too small, increase L_IWORK to 7'
        IFAIL=10
      ENDIF

      RETURN
      END


      SUBROUTINE AM_DE(AII,AIO,CONTROL,ERROR_TYPE,IFAIL,ISNOLD,K,
     '  KOLD,L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS,
     '  MAX_ITERS,MAX_ORDER,MAX_RND,MODEL,NUMBER_EQN,NUMBER_STEPS,
     '  ABS_ERR,ALPHA,ARI,ARO,BETA,DELSGN,DY,DYOUT,ERROR_WEIGHT,G,
     '  H,HOLD,MAX_STEP,PARAMETERS,PHI,PREDICTION,PSI,REL_ERR,SIGMA,
     '  T,TT,TOLD,TOUT,V,W,Y,YY,EXTEND_INTERVAL,PHASE1,ROUND_CTRL,
     '  START,STIFF_EQNS,FUNC)

C#### Subroutine: AM_DE
C###  Description:
C###    Adams-Moulton differential equation integrator. Called from
C###    the ADAMS_MOULTON buffer subroutine.
      
      IMPLICIT NONE
      
!     Parameter list
      INTEGER L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS
      INTEGER AII(L_AII),AIO(L_AIO),CONTROL(L_CONTROL),ERROR_TYPE,
     '  IFAIL,ISNOLD,K,KOLD,MAX_ITERS,MAX_ORDER,MAX_RND,MODEL(L_MODEL),
     '  NUMBER_EQN,NUMBER_STEPS
      REAL*8 ABS_ERR,ALPHA(MAX_ORDER),ARI(L_ARI),ARO(L_ARO),
     '  BETA(MAX_ORDER),DELSGN,DY(NUMBER_EQN),DYOUT(NUMBER_EQN),
     '  ERROR_WEIGHT(NUMBER_EQN),G(MAX_ORDER+1),H,HOLD,MAX_STEP,
     '  PARAMETERS(L_PARAMETERS),PHI(NUMBER_EQN,MAX_ORDER+2+MAX_RND),
     '  PREDICTION(NUMBER_EQN),PSI(MAX_ORDER),REL_ERR,
     '  SIGMA(MAX_ORDER+1),T,TT,TOLD,TOUT,V(MAX_ORDER),W(MAX_ORDER),
     '  Y(NUMBER_EQN),YY(NUMBER_EQN)
      LOGICAL EXTEND_INTERVAL,PHASE1,ROUND_CTRL,START,STIFF_EQNS
      EXTERNAL FUNC
!     Local Variables
      INTEGER ERR_CODE,ISN,NUM_TIMES_LOW_ORDER,L,IT_NUM
      REAL*8 ABS_DEL,ABS_EPS,DEL,DLAMCH,EPS,EPSILON,FOUR_EPSILON,
     '  REL_EPS,TEND
      LOGICAL CRASH,FINISHED

C???DB.  Where does DLAMCH come from ?
C      EPSILON=DLAMCH('EPS')
      FOUR_EPSILON=4.0d0*EPSILON

C     Test for improper parameters
      EPS=DMAX1(REL_ERR,ABS_ERR)
      IFAIL=IABS(IFAIL)
      IF(NUMBER_EQN.LT.1.OR.
     '  T.EQ.TOUT.OR.
     '  REL_ERR.LT.0.0d0.OR.ABS_ERR.LT.0.0d0.OR.EPS.LE.0.0d0.OR.
     '  (IFAIL.LT.1.OR.IFAIL.GT.5).OR.
     '  (IFAIL.NE.1.AND.T.NE.TOLD).OR.
     '  (ERROR_TYPE.LT.1.OR.ERROR_TYPE.GT.4).OR.
     '  (MAX_ORDER.LT.1.OR.MAX_ORDER.GT.12)) THEN
        IFAIL = 6
      ELSE

C       On each call set interval of integration and counter for the
C       number of steps.
        DEL=TOUT-T
        ABS_DEL=DABS(DEL)
        TEND=T+10.0d0*DEL
        ISN=ISIGN(1,IFAIL)
        IF(ISN.LT.0) TEND = TOUT
        IT_NUM=0
        NUM_TIMES_LOW_ORDER=0
        STIFF_EQNS=.FALSE.
        REL_EPS=REL_ERR/EPS
        ABS_EPS=ABS_ERR/EPS
        IF(IFAIL.EQ.1.OR.ISNOLD.LT.0.OR.DELSGN*DEL.LE.0.0d0) THEN
C         On start and restart also set the work variables TT and YY,
C         store the direction of integration and initialise the step
C         size
          START=.TRUE.
          TT=T
          DO l=1,NUMBER_EQN
            YY(l)=Y(l)
          ENDDO !l
          DELSGN=DSIGN(1.0d0,DEL)
          H=DSIGN(DMAX1(DABS(TOUT-TT),FOUR_EPSILON*DABS(TT)),TOUT-TT)
          IF(H.GT.MAX_STEP) H=MAX_STEP
        ENDIF

C       Iterate on the solution until finished.
        FINISHED=.FALSE.
        DO WHILE(.NOT.FINISHED)

          IF(DABS(TT-T).GE.ABS_DEL) THEN
C           If already past the output point then interpolate and 
C           finish
            CALL AM_INTERPOLATE(KOLD,MAX_ORDER,MAX_RND,
     '        NUMBER_EQN,DYOUT,PHI,PSI,TT,TOUT,YY,Y)
            IFAIL=2
            T=TOUT
            TOLD=T
            ISNOLD=ISN
            FINISHED=.TRUE.
          ELSE IF(.NOT.EXTEND_INTERVAL.AND.
     '        DABS(TOUT-TT).LT.FOUR_EPSILON*DABS(TT)) THEN
C           If you cannot extend the interval to go past the output
C           output point and you are sufficiently close then
C           extroplate and finish.
            H=TOUT-TT
            CALL FUNC(TT,YY,DY,AII,AIO,CONTROL,L_AII,L_AIO,
     '        L_ARI,L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS,MODEL,
     '        NUMBER_EQN,ARI,ARO,PARAMETERS,ERR_CODE)
            IF(ERR_CODE.EQ.0) THEN
              DO l=1,NUMBER_EQN
                Y(l)=YY(l)+H*DY(l)
              ENDDO !l
              IFAIL=2
              T=TOUT
              TOLD=T
              ISNOLD=ISN
              FINISHED=.TRUE.
            ELSE
              IFAIL=7
              FINISHED=.TRUE.
            ENDIF
          ELSE IF(IT_NUM.GE.MAX_ITERS) THEN
C           Test for too many steps            
            IFAIL=ISN*4
            IF(STIFF_EQNS) IFAIL=ISN*5
            DO l=1,NUMBER_EQN
              Y(l)=YY(l)
            ENDDO !l
            T=TT
            TOLD=T
            ISNOLD=1
            FINISHED=.TRUE.
          ELSE
C           Limit step size, set the error weight vector and take 
C           a step
            H=DSIGN(DMIN1(DABS(H),DABS(TEND-TT)),H)
            IF(H.GT.MAX_STEP) H=MAX_STEP
            ERR_CODE=0
            IF(ERROR_TYPE.EQ.1) THEN
              DO l=1,NUMBER_EQN
                ERROR_WEIGHT(l)=1.0d0
              ENDDO !l
            ELSE IF(ERROR_TYPE.EQ.2) THEN              
              DO l=1,NUMBER_EQN
                ERROR_WEIGHT(l)=DABS(YY(l))
                IF(DABS(YY(l)).LE.EPSILON) ERR_CODE=1
              ENDDO !l
            ELSE IF(ERROR_TYPE.EQ.3) THEN
              DO l=1,NUMBER_EQN
                ERROR_WEIGHT(l)=DABS(DY(l))
                IF(DABS(DY(l)).LE.EPSILON) ERR_CODE=1
              ENDDO !l
            ELSE IF(ERROR_TYPE.EQ.4) THEN
              DO l=1,NUMBER_EQN
                ERROR_WEIGHT(l)=REL_EPS*DABS(YY(l))+ABS_EPS
              ENDDO !l
            ENDIF
            IF(ERR_CODE.EQ.0) THEN
            
              CALL AM_STEP(AII,AIO,CONTROL,ERR_CODE,K,KOLD,L_AII,
     '          L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS,
     '          MAX_ORDER,MAX_RND,MODEL,NUMBER_EQN,NUMBER_STEPS,
     '          ALPHA,ARI,ARO,BETA,DY,EPS,EPSILON,ERROR_WEIGHT,G,H,
     '          HOLD,MAX_STEP,PARAMETERS,PHI,PREDICTION,PSI,SIGMA,
     '          TT,V,W,YY,CRASH,PHASE1,ROUND_CTRL,START,FUNC)

              IF(ERR_CODE.EQ.0) THEN
                IF(CRASH) THEN
C                 Tolerances too small
                  IFAIL=ISN*3
                  REL_ERR=EPS*REL_EPS
                  ABS_ERR=EPS*ABS_EPS
                  DO l=1,NUMBER_EQN
                    Y(l)=YY(l)
                  ENDDO !l
                  T=TT
                  TOLD=T
                  ISNOLD=1
                  FINISHED=.TRUE.
                ELSE 
C                 Adjust number of steps and test for stiffness
                  IT_NUM=IT_NUM+1
                  NUM_TIMES_LOW_ORDER=NUM_TIMES_LOW_ORDER+1
                  IF(KOLD.GT.4) NUM_TIMES_LOW_ORDER=0
                  IF(NUM_TIMES_LOW_ORDER.GE.50) STIFF_EQNS=.TRUE.
                ENDIF
              ELSE
C               Error from func
                IFAIL=7
                FINISHED=.TRUE.
              ENDIF
            ELSE
C             Error weight is zero
              IFAIL=8
              FINISHED=.TRUE.
            ENDIF
          ENDIF          
        ENDDO
      ENDIF

      RETURN
      END


      SUBROUTINE AM_INTERPOLATE(K,MAX_ORDER,MAX_RND,
     '  NUMBER_EQN,DYOUT,PHI,PSI,T,TOUT,Y,YOUT)

C#### Subroutine: AM_INTERPOLATE
C###  Description:
C###    Interpolates the solution at TOUT using the Kth order Adams
C###    polynomial calculated near T in AM_STEP. On output YOUT
C###    contains the solution at TOUT and DYOUT contains the 
C###    derivative of the solution at TOUT.

      IMPLICIT NONE
!     Parameter list
      INTEGER K,MAX_ORDER,MAX_RND,NUMBER_EQN
      REAL*8 PHI(NUMBER_EQN,MAX_ORDER+2+MAX_RND),PSI(MAX_ORDER),T,
     '  TOUT,Y(NUMBER_EQN),YOUT(NUMBER_EQN),DYOUT(NUMBER_EQN)
!     Local Variables
      INTEGER i,j,l
      REAL*8 ETA,G(13),GAMMA,H_I,RHO(13),TERM,W(13)

      DATA G(1) /1.0d0/
      DATA RHO(1) /1.0d0/

C     The formula for interpolation is
C       y_out=y_(n+1)+h_I\sum{i=1}{K+1}[g_i,1^I.Phi_i(n+1)]
C       dy_out=\sum{i=1}{K+1}[rho_i^I.Phi_i(n+1)]
C     where
C       h_I=t_out-t_(n+1)
C       g_i,q^I=1/q                                       i=1
C       g_i,q^I=GAMMA_(I-1)(1).g_(i-1),q^I-
C         eta_(i-1).g_(i-1),(q+1)^I                       i>=2
C       eta_i=h_I/Psi_i(n+1)
C       GAMMA_i(s)=s.h_I/Psi_1(n+1)                       i=1
C       GAMMA_i(s)=(s.h_I+Psi_(i-1)(n+1))/Psi_i(n+1)      i>=2
C       rho_1=1.0                                         i=1
C       rho_i=rho_(i-1).GAMMA_(i-1)(1)                    i=2,..,K+1

      H_I=TOUT-T

C     Initialise W for computing G
      DO i=1,K+1
        W(i)=1.0d0/DBLE(i)
      ENDDO !i
      TERM=0.0d0

C     Compute G
      DO j=2,K+1
        GAMMA=(H_I+TERM)/PSI(j-1)
        ETA=H_I/PSI(j-1)
        DO i=1,K+2-j
          W(i)=GAMMA*W(i)-ETA*W(i+1)
        ENDDO !i
        G(j)=W(1)
        RHO(j)=GAMMA*RHO(j-1)
        TERM=PSI(j-1)
      ENDDO !j

C     Interpolate
      DO l=1,NUMBER_EQN
        YOUT(l)=0.0d0
        DYOUT(l)=0.0d0
      ENDDO !l
      DO j=1,K+1
        i=K+2-j
        DO l=1,NUMBER_EQN
          YOUT(l)=YOUT(l)+G(i)*PHI(l,i)
          DYOUT(l)=DYOUT(l)+RHO(i)*PHI(l,i)
        ENDDO !l
      ENDDO !j
      DO l=1,NUMBER_EQN
        YOUT(l)=Y(l)+H_I*YOUT(l)
      ENDDO !l
      
      RETURN
      END


      SUBROUTINE AM_STEP(AII,AIO,CONTROL,ERR_CODE,K,KOLD,L_AII,
     '  L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS,MAX_ORDER,
     '  MAX_RND,MODEL,NUMBER_EQN,NUMBER_STEPS,ALPHA,ARI,ARO,BETA,
     '  DY,EPS,EPSILON,ERROR_WEIGHT,G,H,HOLD,MAX_STEP,PARAMETERS,PHI,
     '  PREDICTION,PSI,SIGMA,T,V,W,Y,CRASH,PHASE1,ROUND_CTRL,START,
     '  FUNC)

C#### Subroutine: AM_STEP
C###  Description:
C###    Subroutine to perfom the Adams-Moulton step from T to T+H
      
      IMPLICIT NONE
!     Parameter list
      INTEGER L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS
      INTEGER AII(L_AII),AIO(L_AIO),CONTROL(L_CONTROL),ERR_CODE,
     '  K,KOLD,MAX_ORDER,MAX_RND,MODEL(L_MODEL),NUMBER_EQN,NUMBER_STEPS
      REAL*8 ALPHA(MAX_ORDER),ARI(L_ARI),ARO(L_ARO),BETA(MAX_ORDER),
     '  DY(NUMBER_EQN),EPS,EPSILON,ERROR_WEIGHT(NUMBER_EQN),
     '  G(MAX_ORDER+1),H,HOLD,PREDICTION(NUMBER_EQN),MAX_STEP,
     '  PARAMETERS(L_PARAMETERS),PHI(NUMBER_EQN,MAX_ORDER+2+MAX_RND),
     '  PSI(MAX_ORDER),SIGMA(MAX_ORDER+1),T,V(MAX_ORDER),W(MAX_ORDER),
     '  Y(NUMBER_EQN)
      LOGICAL CRASH,PHASE1,ROUND_CTRL,START
      EXTERNAL FUNC
!     Local Variables
      INTEGER i,IFAIL,iq,j,K_MINUS1,K_MINUS2,KNEW,K_PLUS1,K_PLUS2,l,
     '  NUMBER_STEPS_MINUS2,NUMBER_STEPS_PLUS1,NUMBER_STEPS_PLUS2
      REAL*8  ABSH,ERROR_K,ERROR_K_MINUS1,ERROR_K_MINUS2,
     '  ERROR_K_PLUS1,ERR,FOUR_EPSILON,GAMMASTAR(13),HALF_EPS,
     '  HNEW,R,RHO,ROUND,SUM,TAU,TEMP1,TEMP2,TOLD,POWERTWO(13),
     '  TWO_EPSILON
      LOGICAL GOODSTEP

C     Gamma*_(i) is defined to be:
C       Gamma*_(i)=1/i! \int{0}{1}[(s-1)(s)(s+1)...(s+i-2)]ds i=1,2,3,..
C     with Gamma*_(0)=1, but can be generated with the recursion 
C     formula:
C       Gamma*_(m)+1/2.Gamma*_(m-1)+1/3.Gamma*_(m-2)+....+
C       1/(m+1).Gamma*_(0)=0
C     NOTE: For all the cases below Gamma* is negative however the
C     absolute value is stored as we are only interested in the 
C     absolute error.
      REAL*8 GAMMASTAR_1,GAMMASTAR_2,GAMMASTAR_3,GAMMASTAR_4,
     '  GAMMASTAR_5,GAMMASTAR_6,GAMMASTAR_7,GAMMASTAR_8,GAMMASTAR_9,
     '  GAMMASTAR_10,GAMMASTAR_11,GAMMASTAR_12,GAMMASTAR_13
      PARAMETER(GAMMASTAR_1=1.0d0/2.0d0,GAMMASTAR_2=1.0d0/12.0d0,
     '  GAMMASTAR_3=1.0d0/24.0d0,GAMMASTAR_4=19.0d0/720.0d0,
     '  GAMMASTAR_5=27.0d0/1440.0d0,GAMMASTAR_6=863.0d0/60480.0d0,
     '  GAMMASTAR_7=275.0d0/24192.0d0,GAMMASTAR_8=33953.0d0/3628800.0d0,
     '  GAMMASTAR_9=8183.0d0/1036800.0d0,
     '  GAMMASTAR_10=3250433.0d0/479001600.0d0,
     '  GAMMASTAR_11=4671.0d0/788480.0d0,
     '  GAMMASTAR_12=5852897.0d0/1117670400.0d0,
     '  GAMMASTAR_13=78418523.0d0/16765056000.0d0)
      DATA GAMMASTAR / GAMMASTAR_1,GAMMASTAR_2,GAMMASTAR_3,GAMMASTAR_4,
     '  GAMMASTAR_5,GAMMASTAR_6,GAMMASTAR_7,GAMMASTAR_8,GAMMASTAR_9,
     '  GAMMASTAR_10,GAMMASTAR_11,GAMMASTAR_12,GAMMASTAR_13 /

      DATA POWERTWO /2.0d0,4.0d0,8.0d0,16.0d0,32.0d0,64.0d0,128.0d0,
     '  256.0d0,512.0d0,1024.0d0,2048.0d0,4096.0d0,8192.0d0/


      TWO_EPSILON=2.0d0*EPSILON
      FOUR_EPSILON=4.0d0*EPSILON

C     *** BLOCK 0 ***
C     Check if the step size or error tolerance is too small for the
C     machine precision. If it is the first step then initialise the
C     PHI array and estimate a starting step size.

      CRASH=.TRUE.
      IF(DABS(H).LT.FOUR_EPSILON*DABS(T)) THEN
C       If step size is too small the determine an acceptable one and
C       exit
        H=DSIGN(FOUR_EPSILON*DABS(T),H)
        IF(H.GT.MAX_STEP) H=MAX_STEP
      ELSE
        HALF_EPS=EPS/2.0d0
C
        ROUND=0.0d0
        DO l=1,NUMBER_EQN
          ROUND=ROUND+(Y(l)/ERROR_WEIGHT(l))**2
        ENDDO !l
        ROUND=TWO_EPSILON*DSQRT(ROUND)
        IF(HALF_EPS.LT.ROUND) THEN
C         If error tolerance is too small then increase it to an
C         acceptable value and exit
          EPS=2.0d0*ROUND*(1.0d0+FOUR_EPSILON)
        ELSE
          CRASH=.FALSE.
          G(1)=1.0d0
          G(2)=0.5d0
          SIGMA(1)=1.0d0
          
          IF(START) THEN
C           Initialise and compute appropriate step size for the first
C           step
            CALL FUNC(T,Y,DY,AII,AIO,CONTROL,L_AII,L_AIO,L_ARI,
     '        L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS,MODEL,NUMBER_EQN,
     '        ARI,ARO,PARAMETERS,ERR_CODE)
            IF(ERR_CODE.EQ.0) THEN
              SUM=0.0d0
              DO l=1,NUMBER_EQN
                PHI(l,1)=DY(l)
                PHI(l,2)=0.0d0
                SUM=SUM+(DY(l)/ERROR_WEIGHT(l))**2
              ENDDO !l
              SUM=DSQRT(SUM)
              IF(EPS.LT.16.0d0*SUM*H*H) THEN
                ABSH=0.25d0*DSQRT(EPS/SUM)
              ELSE
                ABSH=DABS(H)
              ENDIF
              H=DSIGN(DMAX1(ABSH,FOUR_EPSILON*DABS(T)),H)
              IF(H.GT.MAX_STEP) H=MAX_STEP
              HOLD=0.0d0
              K=1
              KOLD=0
              START=.FALSE.
              PHASE1=.TRUE.
              IF(MAX_RND.GT.0) THEN
                ROUND_CTRL=.FALSE.
                IF(HALF_EPS.LE.100.0d0*ROUND) THEN
                  ROUND_CTRL=.TRUE.
                  DO l=1,NUMBER_EQN
                    PHI(l,MAX_ORDER+3)=0.0d0
                  ENDDO !l
                ENDIF
              ELSE
                ROUND_CTRL=.FALSE.
              ENDIF
            ENDIF
          ENDIF
C         *** End BLOCK 0 ***

          IF(ERR_CODE.EQ.0) THEN
            IFAIL=0
          
C           *** BLOCK 1 ***
C           Compute the coefficients of the formulas for this step.
C           Avoid computing those quantities not changed when the 
C           step size is not changed.   

            GOODSTEP=.FALSE.
            DO WHILE(.NOT.GOODSTEP.AND..NOT.CRASH.AND.ERR_CODE.EQ.0)
            
              K_PLUS1=K+1
              K_PLUS2=K+2
              K_MINUS1=K-1
              K_MINUS2=K-2

C             NUMBER_STEPS is the number of steps taken with step size
C             H. When the current order K is less than NUMBER_STEPS
C             then no coefficients change.

              IF(H.NE.HOLD) NUMBER_STEPS = 0
              IF(NUMBER_STEPS.LE.KOLD) NUMBER_STEPS=NUMBER_STEPS+1
              NUMBER_STEPS_PLUS1 = NUMBER_STEPS+1
              IF (K.GE.NUMBER_STEPS) THEN
C               Compute those components of ALPHA, BETA, PSI and SIGMA
C               which are changed
C               The formula are:
C                 Psi_i(n+1)=h_(n+1)+h_n+...+h_(n+2-i)      i>=1
C                 Alpha_i(n+1)=h_(n+1)/Psi_i(n+1)           i>=1
C                 Beta_1(n+1)=1.0                           i=1
C                 Beta_i(n+1)=Psi_1(n+1).Psi_2(n+1)...Psi_(i-1)(n+1)/
C                   Psi_1(n).Psi_2(n)...Psi_(i-1)(n)        i>1
C                 Sigma_1(n+1)=1.0                          i=1
C                 Sigma_i(n+1)=h.2h...(i-1)h/
C                   Psi_1(n+1).Psi_2(n+1)...Psi_(i-1)(n+1)  i=>1
                BETA(NUMBER_STEPS)=1.0d0
                ALPHA(NUMBER_STEPS)=1.0d0/DBLE(NUMBER_STEPS)
                TEMP1=H*DBLE(NUMBER_STEPS)
                SIGMA(NUMBER_STEPS_PLUS1)=1.0d0
                DO i=NUMBER_STEPS_PLUS1,K
                  TEMP2=PSI(i-1)
                  PSI(i-1)=TEMP1
                  BETA(i)=BETA(i-1)*PSI(i-1)/TEMP2
                  TEMP1=TEMP2+H
                  ALPHA(i)=H/TEMP1
                  SIGMA(i+1)=DBLE(i)*ALPHA(i)*SIGMA(i)
                ENDDO !i
                PSI(K)=TEMP1

C               Compute the coefficients G
                IF(NUMBER_STEPS.LE.1) THEN
C                 Initialise V and set W
                  DO iq=1,K
                    V(iq)=1.0d0/DBLE(iq*(iq+1))
                    W(iq)=V(iq)
                  ENDDO !iq
                ELSE 
                  IF(K.GT.KOLD) THEN
C                   If the order was raised update the diagonal part
C                   of V
                    V(K)=1.0d0/DBLE(K*K_PLUS1)
                    NUMBER_STEPS_MINUS2=NUMBER_STEPS-2
                    DO j=1,NUMBER_STEPS_MINUS2
                      i=K-j
                      V(i)=V(i)-ALPHA(j+1)*V(i+1)
                    ENDDO !J
                  ENDIF
C                 Update V and set W
                  DO iq=1,K_PLUS1-NUMBER_STEPS
                    V(iq)=V(iq)-ALPHA(NUMBER_STEPS)*V(iq+1)
                    W(iq)=V(iq)
                  ENDDO !IQ
                  G(NUMBER_STEPS_PLUS1)=W(1)
C
                ENDIF
C               Compute the G in the work vector W
                NUMBER_STEPS_PLUS2 = NUMBER_STEPS + 2
                DO i=NUMBER_STEPS_PLUS2,K_PLUS1
                  DO iq=1,K_PLUS2-i
                    W(iq)=W(iq)-ALPHA(i-1)*W(iq+1)
                  ENDDO !iq
                  G(i)=W(1)
                ENDDO !i
                
              ENDIF
C             *** End BLOCK 1 ***
     
C             *** BLOCK 2 ***
C             Predicit a solution, PREDICTION, and evaluate the 
C             derivatives, DY, using the predicited solution. 
C             Estimate the local error at order K and the errors
C             at orders K, K-1 and K-2 as if a constant step size
C             were used.

C             Change PHI to PHI* i.e. PHI*_i(n)=BETA_i(n+1).PHI_i(n)
              DO i=NUMBER_STEPS_PLUS1,K
                DO l=1,NUMBER_EQN
                  PHI(l,i)=BETA(i)*PHI(l,i)
                ENDDO !l
              ENDDO !i
              
C             Predicit the solution and differences
              DO l=1,NUMBER_EQN
                PHI(l,K_PLUS2)=PHI(l,K_PLUS1)
                PHI(l,K_PLUS1)=0.0d0
                PREDICTION(l)=0.0d0
              ENDDO !l
              DO j=1,K
                i=K_PLUS1-j
                DO l=1,NUMBER_EQN
                  PREDICTION(l)=PREDICTION(l)+G(i)*PHI(l,i)
                  PHI(l,i)=PHI(l,i)+PHI(l,i+1)
                ENDDO !l
              ENDDO !i
              IF(ROUND_CTRL) THEN
                DO l=1,NUMBER_EQN
                  TAU=H*PREDICTION(l)-PHI(l,MAX_ORDER+3)
                  PREDICTION(l)=Y(l)+TAU
                  PHI(l,MAX_ORDER+4)=(PREDICTION(l)-Y(l))-TAU
                ENDDO !l
              ELSE
                DO l=1,NUMBER_EQN
                  PREDICTION(l)=Y(l)+H*PREDICTION(l)
                ENDDO !l
              ENDIF
              TOLD=T
              T=T+H
              ABSH=DABS(H)
              CALL FUNC(T,PREDICTION,DY,AII,AIO,CONTROL,L_AII,
     '          L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS,
     '          MODEL,NUMBER_EQN,ARI,ARO,PARAMETERS,ERR_CODE)
              IF(ERR_CODE.EQ.0) THEN
C               Estimate errors at orders K,K-1,K-2
C               For a constant step size, h, the local error at x_(n+1)
C               is given by:
C                 ERROR_K=|h.Gamma*_K.Sigma_(K+1)(n+1).Phi_(K+1)^P(n+1)|
                ERROR_K=0.0d0
                DO l=1,NUMBER_EQN
                  ERROR_K=ERROR_K+
     '              ((DY(l)-PHI(l,1))/ERROR_WEIGHT(l))**2
                ENDDO !l
                ERR=ABSH*DSQRT(ERROR_K)*(G(K)-G(K_PLUS1))
                ERROR_K=ABSH*DSQRT(ERROR_K)*SIGMA(K_PLUS1)*
     '            GAMMASTAR(K)
                IF(K_MINUS2.EQ.0) THEN
                  ERROR_K_MINUS1=0.0d0
                  DO l=1,NUMBER_EQN
                    ERROR_K_MINUS1=ERROR_K_MINUS1+
     '                ((DY(l)+PHI(L,K)-PHI(l,1))/ERROR_WEIGHT(l))**2
                  ENDDO !l
                  ERROR_K_MINUS1=ABSH*SIGMA(K)*GAMMASTAR(K_MINUS1)*
     '              DSQRT(ERROR_K_MINUS1)
                ELSE IF(K_MINUS2.GT.0) THEN
                  ERROR_K_MINUS2=0.0d0
                  ERROR_K_MINUS1=0.0d0
                  DO l=1,NUMBER_EQN
                    ERROR_K_MINUS2=ERROR_K_MINUS2+
     '                ((DY(l)+PHI(L,K_MINUS1)-PHI(l,1))/
     '                ERROR_WEIGHT(l))**2
                    ERROR_K_MINUS1=ERROR_K_MINUS1+ 
     '                ((DY(l)+PHI(L,K)-PHI(l,1))/ERROR_WEIGHT(l))**2
                  ENDDO !l
                  ERROR_K_MINUS2=ABSH*SIGMA(K_MINUS1)*
     '              GAMMASTAR(K_MINUS2)*DSQRT(ERROR_K_MINUS2)
                  ERROR_K_MINUS1=ABSH*SIGMA(K)*GAMMASTAR(K_MINUS1)*
     '              DSQRT(ERROR_K_MINUS1)
                ENDIF
                KNEW=K

C               Test if order should be lowered                
                IF(K_MINUS2.EQ.0) THEN
                  IF(ERROR_K_MINUS1.LE.0.5D0*ERROR_K) KNEW = K_MINUS1
                ELSE IF(K_MINUS2.GT.0) THEN
                  IF(DMAX1(ERROR_K_MINUS1,ERROR_K_MINUS2).LE.ERROR_K) 
     '              KNEW = K_MINUS1
                ENDIF
C               *** End BLOCK 2 ***
        
C               Test if the step was successful
                IF(ERR.GT.EPS) THEN

C                 *** BLOCK 3 ***
C                 The step is unsuccessful. 

C                 Restore T, PHI and PSI
                  PHASE1=.FALSE.
                  T=TOLD
                  DO i=1,K
                    DO l=1,NUMBER_EQN
                      PHI(l,i)=(PHI(l,i)-PHI(l,i+1))/BETA(i)
                    ENDDO !l
                  ENDDO !i
                  IF(K.GE.2) THEN
                    DO i=2,K
                      PSI(i-1)=PSI(i)-H
                    ENDDO !i
                  ENDIF

                  IFAIL=IFAIL+1
C                 Double the step size. If the step fails three times
C                 set the order to 1, thereafter use optimal step size.
C                 This procedure will exit if the estimated step size
C                 is too small for the machine precision.

                  IF(IFAIL.LT.3) THEN
                    H=H/2.0d0
                  ELSE IF(IFAIL.EQ.3) THEN
                    KNEW = 1
                    H=H/2.0d0
                  ELSE 
                    KNEW = 1
                    IF(HALF_EPS.LT.ERROR_K/4.0d0) THEN
                      H=DSQRT(HALF_EPS/ERROR_K)*H
                    ELSE
                      H=H/2.0d0
                    ENDIF
                  ENDIF
                  K = KNEW
                  IF(DABS(H).LT.FOUR_EPSILON*DABS(T)) THEN
                    CRASH =.TRUE.
                    H=DSIGN(FOUR_EPSILON*DABS(T),H)
                    IF(H.GT.MAX_STEP) H=MAX_STEP
                    EPS=EPS+EPS                
                  ENDIF
C                 *** End BLOCK 3 ***
              
                ELSE
                  GOODSTEP=.TRUE.
                ENDIF
              ENDIF
              
            ENDDO
            
            IF(.NOT.CRASH.AND.ERR_CODE.EQ.0) THEN
      
C             *** BLOCK 4 ***
C             The step is successfull. Correct the predicited solution,
C             evaluate the derivatives using the correct solution and
C             update the differences. Determine best order and step
C             size for the next step.

              KOLD=K
              HOLD=H
C             Correct and Evaluate
              IF(ROUND_CTRL) THEN
                DO l=1,NUMBER_EQN
                  RHO=H*G(K_PLUS1)*(DY(l)-PHI(l,1))-PHI(l,MAX_ORDER+4)
                  Y(l)=PREDICTION(l)+RHO
                  PHI(l,MAX_ORDER+3)=(Y(l)-PREDICTION(l))-RHO
                ENDDO !l
              ELSE
                DO l=1,NUMBER_EQN
                  Y(l)=PREDICTION(l)+H*G(K_PLUS1)*(DY(l)-PHI(l,1))
                ENDDO !l
              ENDIF
              CALL FUNC(T,Y,DY,AII,AIO,CONTROL,L_AII,L_AIO,L_ARI,
     '          L_ARO,L_CONTROL,L_MODEL,L_PARAMETERS,MODEL,
     '          NUMBER_EQN,ARI,ARO,PARAMETERS,ERR_CODE)
              IF(ERR_CODE.EQ.0) THEN
C               Update differences for next step
                DO l=1,NUMBER_EQN
                  PHI(l,K_PLUS1)=DY(l)-PHI(l,1)
                  PHI(l,K_PLUS2)=PHI(l,K_PLUS1)-PHI(l,K_PLUS2)
                ENDDO !l
                DO i=1,K
                  DO l=1,NUMBER_EQN
                    PHI(l,i)=PHI(l,i)+PHI(l,K_PLUS1)
                  ENDDO !l
                ENDDO !k

C               Estimate the error at order K+1 unless: we are in
C               the first phase in which case always raise the order;
C               we have already decieded to lower the order; or the
C               step size is not constant so the error estimate is
C               unreliable.
                ERROR_K_PLUS1=0.0d0
                IF(KNEW.EQ.K_MINUS1.OR.K.EQ.MAX_ORDER) PHASE1=.FALSE.
                IF(PHASE1) THEN 
C                 Raise the order
                  K=K_PLUS1
                  ERROR_K=ERROR_K_PLUS1
                ELSE              
                  IF(KNEW.EQ.K_MINUS1) THEN
C                   Lower the order
                    K=K_MINUS1
                    ERROR_K=ERROR_K_MINUS1                   
                  ELSE IF(K_PLUS1.LE.NUMBER_STEPS) THEN
                    DO l=1,NUMBER_EQN
                      ERROR_K_PLUS1=ERROR_K_PLUS1+
     '                  (PHI(l,K_PLUS2)/ERROR_WEIGHT(l))**2
                    ENDDO !l
                    ERROR_K_PLUS1=ABSH*GAMMASTAR(K_PLUS1)*
     '                DSQRT(ERROR_K_PLUS1)
C                   Using the estimated error at order K+1 determine
C                   the appropriate order for the next step.
                    IF(K.LE.1) THEN
                      IF(ERROR_K_PLUS1.LT.ERROR_K/2.0d0.AND.
     '                  K.NE.MAX_ORDER) THEN
C                       Raise the order
                        K=K_PLUS1
                        ERROR_K=ERROR_K_PLUS1
                      ENDIF
                    ELSE
                      IF(ERROR_K_MINUS1.LE.
     '                  DMIN1(ERROR_K,ERROR_K_PLUS1)) THEN
C                       Lower the order
                        K=K_MINUS1
                        ERROR_K=ERROR_K_MINUS1
                      ELSE IF(ERROR_K_PLUS1.LT.ERROR_K.AND.
     '                    K.NE.MAX_ORDER) THEN
C                       Here the error at K+1 < the error at K < the
C                       maximum error of K-1 and K-2 otherwise the
C                       order would have been lowered in block 2, thus
C                       raise the order
                        K = K_PLUS1
                        ERROR_K = ERROR_K_PLUS1
                      ENDIF
                    ENDIF                
                  ENDIF
                ENDIF

C               With the new order determine the appropriate step
C               size for the next step.
                HNEW=H+H
                IF(.NOT.PHASE1.AND.
     '            HALF_EPS.LT.ERROR_K*POWERTWO(K+1)) THEN
                  HNEW=H
                  IF(HALF_EPS.LT.ERROR_K) THEN
                    R=(HALF_EPS/ERROR_K)**(1.0d0/DBLE(K+1))
                    HNEW=ABSH*DMAX1(0.5d0,DMIN1(0.9d0,R))
                    HNEW=DSIGN(DMAX1(HNEW,FOUR_EPSILON*DABS(T)),H)
                  ENDIF
                ENDIF
                H=HNEW
                IF(H.GT.MAX_STEP) H=MAX_STEP
              ENDIF
C             *** End BLOCK 4 ***
            
            ENDIF
          ENDIF
        ENDIF
      ENDIF

      RETURN
      END


      SUBROUTINE ATR(t,y,F)

C#### Subroutine: ATR
C###  Description: 
C###    Evaluates RHS for ATR system of ODE's

      IMPLICIT NONE
      
      INCLUDE 'cell/cell01.cmn'
! SMAR009 not needed      INCLUDE 'cell/deoxs00.cmn'

      ! Passed variables
      REAL*8 t,y(*),F(*)
      !potentials
      REAL*8 EK,ENa,ECa
      COMMON /ATR_EP/ EK,ENa,ECa
      !currents
      REAL*8 ATR_INa,ATR_ICaL,ATR_It,ATR_Isus,ATR_IKs,ATR_IKr,ATR_IK1,
     '  ATR_IBNa,ATR_IBCa,ATR_INaK,ATR_ICaP,ATR_INaCa,ATR_Idi,ATR_IUP,
     '  ATR_IREL,ATR_Itr
      COMMON /ATR_I/ ATR_INa,ATR_ICaL,ATR_It,ATR_Isus,ATR_IKs,ATR_IKr,
     '  ATR_IK1,ATR_IBNa,ATR_IBCa,ATR_INaK,ATR_ICaP,ATR_INaCa,ATR_Idi,
     '  ATR_IUP,ATR_IREL,ATR_Itr
      !RTF
      REAL*8           RTF
      COMMON /ATR_RTF/ RTF
      !local variables
      REAL*8 V,mNa,h1Na,h2Na,dL,fL1,fL2,r,s,rsus,ssus,n,Pa,Nai,Ki,Cai,
     '  Cad,OC,OTC,OTMgC,OTMgMg,OCalse,Nac,Kc,Cac,CaREL,F1,F2,  !SMAR009 21/12/98 CaUP,
     '  ATR_STIMSIZE,dOdt,r_act,r_inact,

     '  mNa_bar,hNa_bar,dL_bar,fL_bar,r_bar,s_bar,rsus_bar,ssus_bar,
     '  n_bar,Pa_bar, !Nai_bar,
     '  tau_mNa,tau_h1Na,tau_h2Na,tau_dL,tau_fL1,tau_fL2,tau_r,tau_s,
     '  tau_rsus,tau_ssus,tau_n,tau_Pa   !,tau_Nai
 
      ! State variables
      V       = y( 1) !membrane potential
      mNa     = y( 2) !Na channel m gate
      h1Na    = y( 3) !Na channel h1 gate
      h2Na    = y( 4) !Na channel h2 gate
      dL      = y( 5) !CaL Activation gating variable
      fL1     = y( 6) !CaL Fast Inactivation gating variable
      fL2     = y( 7) !CaL Slow Inactivation gating variable
      r       = y( 8) !K Transient Activation gating variable
      s       = y( 9) !K Transient Inactivation gating variable
      rsus    = y(10) !K Sustained Activation gating variable
      ssus    = y(11) !K Sustained Inactivation gating variable
      n       = y(12) !Ks delayed rectifier Activation gate 
      Pa      = y(13) !Kr delayed rectifier Activation gate 
      Nai     = y(14) !Intracellular [Na]
      Ki      = y(15) !Intracellular [K]
      Cai     = y(16) !Intracellular [Ca]
      Cad     = y(17) !Restricted subsarcolemmal [Ca]
      OC      = y(18) !Ca Fractional occupancy in calmodulin buffer
      OTC     = y(19) !Ca Fractional occupancy in troponin-Ca buffer
      OTMgC   = y(20) !Ca Fractional occupancy in troponin-Mg buffer
      OTMgMg  = y(21) !Mg Fractional occupancy in troponin-Mg buffer
      OCalse  = y(22) !Ca Fractional occupancy in calsequestrin buffer
      Nac     = y(23) !Extracellular Cleft space Na Concentrations
      Kc      = y(24) !Extracellular Cleft space K Concentrations
      Cac     = y(25) !Extracellular Cleft space Ca Concentrations
      CaREL   = y(26) !Sarcoplasmic Release Compartment Ca concentration
c      CaUP    = y(27) !Sarcoplasmic Uptake Compartment Ca concentrations
      F1      = y(28) !Relative inactive precurser in Irel formulation
      F2      = y(29) !Relative activator in Irel formulation

      ! Equilibrium potentials
      CALL ATR_EQUILIB_POTS(RTF,Nai,Ki,Cai,Nac,Kc,Cac)
      ! EK = RTF*DLOG(Kc/Ki)
      ! ENa = RTF*DLOG(Nac/Nai)
      ! ECa = RTF/2.d0*DLOG(Cac/Cai)

      !ionic currents
      CALL ATR_CURRENTS(y)

      !Set the stimulus SIZE.
      CALL ATR_CHANGE(t,ATR_STIMSIZE) 

      !(assumed ATR_STIMSIZE = ATR_Istim between TPS and TPS+TP)
      ![ 1] dV/dt                                                
      F( 1)  = (ATR_INa+ATR_ICaL+ATR_It+ATR_Isus+ATR_IK1+ATR_IBNa
     '  +ATR_IBCa+ATR_INaK+ATR_ICaP+ATR_INaCa-ATR_STIMSIZE)
     '  /(-1.d0*ATR_Cm)
 
      ![ 2] dm/dt
      mNa_bar = 1.d0/(1.d0+DEXP((V+27.12d0)/-8.21d0))
      tau_mNa = 42.d-6*DEXP(-((V+25.57d0)/28.8d0)**2)+24.d-6
      F( 2)  = (mNa_bar-mNa)/tau_mNa

      ![ 3] dh1/dt                                 
      hNa_bar =  1.d0/(1.d0+DEXP((V+63.6d0)/5.3d0))
      tau_h1Na =  3.d-2/(1.d0+DEXP((V+35.1d0)/3.2d0))+3.d-4
      F( 3)  = (hNa_bar-h1Na)/tau_h1Na
      
      ![ 4] dh2/dt  
      tau_h2Na =  12.d-2/(1.d0+DEXP((V+35.1d0)/3.2d0))+3.d-3
      F( 4)  = (hNa_bar-h2Na)/tau_h2Na                         

      ![ 5] d(dL)/dt                                 
      dL_bar =  1.d0/(1.d0+DEXP((V+9.d0)/-5.8d0))
      tau_dL = 27.d-4*DEXP(-((V+35.d0)/30.d0)**2)+2.d-3
      F( 5)  = (dL_bar-dL)/tau_dL 
 
      ![ 6] dfL1/dt     
      fL_bar = 1.d0/(1.d0+DEXP((V+27.4d0)/7.1d0))
      tau_fL1 =  161.d-3*DEXP(-((V+40.d0)/14.4d0)**2)+1.d-2
      F( 6)  = (fL_bar-fL1)/tau_fL1
      
      ![ 7] dfL2/dt
      tau_fL2 =  1.3323d0*DEXP(-((V+40.d0)/14.4d0)**2)+626.d-4
      F( 7)  = (fL_bar-fL2)/tau_fL2

      ![ 8] dr/dt
      r_bar =  1.d0/(1.d0+DEXP((V-1.d0)/-11.d0))
      tau_r = 35.d-4*DEXP(-(V/30.d0)**2)+15.d-4
      F( 8)  = (r_bar-r)/tau_r 
      
      ![ 9] ds/dt
      s_bar =  1.d0/(1.d0+DEXP((V+40.5d0)/11.5d0))
      tau_s = 4812.d-4*DEXP(-((V+52.45d0)/14.97d0)**2)+1414.d-5
      F( 9)  = (s_bar-s)/tau_s 
      
      ![10] d(rsus)/dt
      rsus_bar =  1.d0/(1.d0+DEXP((V+4.3d0)/-8.d0))
      tau_rsus = 9.d-3/(1.d0+DEXP((V+5.d0)/12.d0))+5.d-4
      F(10)  = (rsus_bar-rsus)/tau_rsus 

      ![11] d(ssus)/dt
      ssus_bar =  4.d-1/(1.d0+DEXP((V+20.d0)/10.d0))+6.d-1
      tau_ssus = 47.d-3/(1.d0+DEXP((V+60.d0)/10.d0))+3.d-1
      F(11)  = (ssus_bar-ssus)/tau_ssus 
      
      ![12] dn/dt
      n_bar =  1.d0/(1.d0+DEXP((V-19.d0)/-12.7d0))
      tau_n = 7.d-1+4.d-1*DEXP(-((V-20.d0)/20.d0)**2)
      F(12)  = (n_bar-n)/tau_n

      ![13] dPa/dt
      Pa_bar =  1.d0/(1.d0+DEXP((V+15.d0)/-6.d0))
      tau_Pa =  3118.d-5+21718.d-5*DEXP(-((V+20.1376d0)/22.1996d0)**2)
      F(13) = (Pa_bar-Pa)/tau_Pa

      ![14] d[Na]i/dt
      F(14) = -(ATR_INa+ATR_IBNa+3.d0*(ATR_INaK+ATR_INaCa)+ATR_QNaen)
     '  /(ATR_Voli*ATR_Faraday)

      ![15] d[K]i/dt
      F(15) = (ATR_It+ATR_Isus+ATR_IK1+ATR_IKs+ATR_IKr-2.d0*ATR_INaK)
     '  /(ATR_Voli*ATR_Faraday)

      ![18] dOCdt
      F(18) = 2.d5*Cai*(1.d0-OC)-476.d0*OC

      ![19] dOTCdt
      F(19) = 784.d2*Cai*(1.d0-OTC)-392.d0*OTC

      ![20] dOTMgCdt
      F(20) = 2.D5*Cai*(1.d0-OTMgC-OTMgMg)-6.6d0*OTMgC

      ![21] dOTMgMgdt
      F(21) = 2.D3*ATR_Mgi*(1.d0-OTMgC-OTMgMg)-666d0*OTMgMg
  
      ![16] d[Ca]i/dt
      dOdt = 8.d-2*F(19)+16.d-2*F(20)+45.d-3*F(18) !dO/dt 
      F(16) = (ATR_Idi-ATR_IBCa-ATR_ICaP+2.d0*ATR_INaCa-ATR_IUP+
     '  ATR_IREL)
     '  /(2.d0*ATR_Voli*ATR_Faraday)-dOdt
   
      ![17] d[Ca]d/dt
      F(17) =-(ATR_ICaL-ATR_Idi)/(2.d0*ATR_Vold*ATR_Faraday)

      ![22] dOCalse/dt
      F(22) = 480.d0*CaREL*(1.d0-OCalse)-4.d2*OCalse
      
      ![23] d[Na]c/dt
      F(23) = (ATR_Nab-Nac)/ATR_tau_Na
     '  +(ATR_INa+ATR_IBNa+3.d0*(ATR_INaK+ATR_INaCa)+ATR_QNaen)
     '  /(ATR_Volc*ATR_Faraday)

      ![24] d[K]c/dt
      F(24) = (ATR_Kb-Kc)/ATR_tau_K
     '  +(ATR_It+ATR_Isus+ATR_IK1+ATR_IKs+ATR_IKr-2.d0*ATR_INaK)
     '  /(ATR_Volc*ATR_Faraday)

      ![25] d[Ca]c/dt
      F(25) = (ATR_Cab-Cac)/ATR_tau_Ca
     '  +(ATR_ICaL+ATR_IBCa+ATR_ICaP-2.d0*ATR_INaCa)
     '  /(2.d0*ATR_Volc*ATR_Faraday)

      ![26] d[Ca]REL/dt
      F(26) = (ATR_Itr-ATR_IREL)/(2.d0*ATR_VolREL*ATR_Faraday)
     '  -31.d0*F(22) !(F(22)=dOCalse/dt)
  
      ![27] d[Ca]UP/dt
      F(27) = (ATR_IUP-ATR_Itr)/(2.d0*ATR_VolUP*ATR_Faraday)

      ![28] dF1/dt
      r_act = 203.8d0*((Cai/(Cai+ATR_KRELi))**4
     '  +(Cad/(Cad+ATR_KRELd))**4)
      F(28) = ATR_rrecov*(1.d0-F1-F2)-r_act*F1

      ![29] dF2/dt
      r_inact = 33.96d0+33.96d0*(Cai/(Cai+ATR_KRELi))
      F(29) = r_Act*F1-r_inact*F2
      
      RETURN
      END


      SUBROUTINE ATR_CHANGE(t,ATR_STIMSIZE)

C#### Subroutine: ATR_CHANGE
C###  Description:
C###    Sets the stimulus size depending on the time
      
      IMPLICIT NONE
      
      INCLUDE 'cell/cell01.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      !Passed variables
      REAL*8 t,ATR_STIMSIZE
      !local variables
c      REAL*8 PERIOD,stimulation_time

      !Set the stimulus curent
      IF (t.GE.TPS.AND.t.LE.TPS+TP) THEN
        ATR_STIMSIZE = ATR_Istim
      ELSE
        ATR_STIMSIZE = 0.0d0
      ENDIF
      
      !Evaluate the new stimulus time
c      IF(STIM_FREQ.GT.0.0d0) THEN
c        IF (t.GT.TPS+TP) THEN
c          PERIOD = 1.0d0 / (STIM_FREQ*1.0d-3) !freq in Hz (1/s -> 1/ms)
c          TPS = TPS + PERIOD
c        ENDIF
c      ENDIF
      
      RETURN
      END


      SUBROUTINE ATR_CURRENTS(y)
      
C#### Subroutine: ATR_CURRENTS
C###  Description:
C###    Evaluate currents for ATR model.

      IMPLICIT NONE
      
      INCLUDE 'cell/cell01.cmn'
   
      ! Passed variables
      REAL*8 y(*)
      !Potentials
      REAL*8 EK,ENa,ECa
      COMMON /ATR_EP/ EK,ENa,ECa
      !currents
      REAL*8 ATR_INa,ATR_ICaL,ATR_It,ATR_Isus,ATR_IKs,ATR_IKr,ATR_IK1,
     '  ATR_IBNa,ATR_IBCa,ATR_INaK,ATR_ICaP,ATR_INaCa,ATR_Idi,ATR_IUP,
     '  ATR_IREL,ATR_Itr
      COMMON /ATR_I/ ATR_INa,ATR_ICaL,ATR_It,ATR_Isus,ATR_IKs,ATR_IKr,
     '  ATR_IK1,ATR_IBNa,ATR_IBCa,ATR_INaK,ATR_ICaP,ATR_INaCa,ATR_Idi,
     '  ATR_IUP,ATR_IREL,ATR_Itr
      !RTF
      REAL*8           RTF
      COMMON /ATR_RTF/ RTF
      !local variables
      REAL*8 V,mNa,h1Na,h2Na,dL,fL1,fL2,r,s,rsus,ssus,n,Pa,Nai,Cai,
     '  Cad,Nac,Kc,Cac,CaREL,CaUP,F2,
     '  IINa,VRTF,fCa,P_i
! SMAR009 21/12/98 (not used here) OC,OTC,OTMgC,OTMgMg,OCalse,Ki,F1,

      ! State variables
      V       = y( 1) !membrane potential
      mNa     = y( 2) !Na channel m gate
      h1Na    = y( 3) !Na channel h1 gate
      h2Na    = y( 4) !Na channel h2 gate
      dL      = y( 5) !CaL Activation gating variable
      fL1     = y( 6) !CaL Fast Inactivation gating variable
      fL2     = y( 7) !CaL Slow Inactivation gating variable
      r       = y( 8) !K Transient Activation gating variable
      s       = y( 9) !K Transient Inactivation gating variable
      rsus    = y(10) !K Sustained Activation gating variable
      ssus    = y(11) !K Sustained Inactivation gating variable
      n       = y(12) !Ks delayed rectifier Activation gate 
      Pa      = y(13) !Kr delayed rectifier Activation gate 
      Nai     = y(14) !Intracellular [Na]
c      Ki      = y(15) !Intracellular [K]
      Cai     = y(16) !Intracellular [Ca]
      Cad     = y(17) !Restricted subsarcolemmal [Ca]
c      OC      = y(18) !Ca Fractional occupancy in calmodulin buffer
c      OTC     = y(19) !Ca Fractional occupancy in troponin-Ca buffer
c      OTMgC   = y(20) !Ca Fractional occupancy in troponin-Mg buffer
c      OTMgMg  = y(21) !Mg Fractional occupancy in troponin-Mg buffer
c      OCalse  = y(22) !Ca Fractional occupancy in calsequestrin buffer
      Nac     = y(23) !Extracellular Cleft space Na Concentrations
      Kc      = y(24) !Extracellular Cleft space K Concentrations
      Cac     = y(25) !Extracellular Cleft space Ca Concentrations
      CaREL   = y(26) !Sarcoplasmic Release Compartment Ca concentration
      CaUP    = y(27) !Sarcoplasmic Uptake Compartment Ca concentrations
c      F1      = y(28) !Relative inactive precurser in Irel formulation
      F2      = y(29) !Relative activator in Irel formulation
      
      VRTF = V / RTF ! VF/RT (dimensionless)
      
      !Na current
      IINa = ATR_Faraday/RTF*((DEXP((V-ENa)/RTF)-1.d0)
     '  /(DEXP(VRTF)-1.d0))
      ATR_INa = ATR_PNa*mNa**3*(9.d-1*h1Na*1.d-1*h2Na)*Nac*V*IINa

      !L_Type Ca current
      fCa = Cad/(Cad+ATR_KCa)
      ATR_ICaL = ATR_GCaL*dL*(fCa*fL1+(1.d0-fCa)*fL2)*(V-ATR_ECaapp)

      !Transient Outward K current
      ATR_It = ATR_Gt*r*s*(V-EK)

      !Sustained Outward K current
      ATR_Isus = ATR_Gsus*rsus*ssus*(V-EK)

      !Delayed Rectifier K currents
      ATR_IKs = ATR_GKs*n*(V-EK) 
      P_i = 1.d0/(1.d0+DEXP((V+55.d0)/24.d0)) 
      ATR_IKr = ATR_GKr*Pa*P_i*(V-EK) 

      !Inward Rectifier K current
      ATR_IK1 = ATR_GK1*Kc**(4457.d-4)
     '  *((V-EK)/(1.d0+DEXP((V-EK+3.6d0)/RTF)))

      !Background Inward currents
      ATR_IBNa = ATR_GBNa*(V-ENa) 
      ATR_IBCa = ATR_GBCa*(V-ECa)

      !Pump curents nakcap
      ATR_INaK = ATR_INaK_max*(Kc/(Kc+ATR_KNaKK))
     '  *(Nai**(1.5d0)/(Nai**(1.5d0)+ATR_KNaKNa**(1.5d0)))
     '  *(V+150.d0)/(V+200.d0)
      ATR_ICaP = ATR_ICaP_max*(Cai/(Cai+ATR_KCaP))

      !Exchanger current
      ATR_INaCa = ATR_KNaCa*(Nai**3*Cac*DEXP(ATR_gamma*VRTF)
     '  -Nac**3*Cai*DEXP((ATR_gamma-1)/VRTF))
     '  /(1.d0+ATR_dNaCa*(Nac**3*Cai+Nac**3*Cac))

      !Diffusion current Ca (from subsarcolemmal space to cystol)
      ATR_Idi = (Cad-Cai)*2.d0*ATR_Faraday*ATR_Vold/ATR_tau_di

      !Uptake Ca current (Sarcoplasmic reticulum)
      ATR_IUP = ATR_IUP_max*(Cai/ATR_Kcyca-ATR_Kxcs**2*CaUP/ATR_Ksrca)
     '  /((Cai+ATR_Kcyca)/ATR_Kcyca+ATR_Kxcs*(CaUP+ATR_Ksrca)/ATR_Ksrca)

      !Translocation Ca current (Sarcoplasmic reticulum)
      ATR_Itr = (CaUP-CaREL)*2.d0*ATR_Faraday*ATR_VolREL/ATR_tau_tr
  
      !Release Ca current (Sarcoplasmic reticulum)
      ATR_IREL = ATR_alphaREL*(F2/(F2+25.d-2))**2*(CaREL-Cai)

      RETURN
      END


      SUBROUTINE ATR_EQUILIB_POTS(RTF,Nai,Ki,Cai,Nac,Kc,Cac)

C#### Subroutine: ATR_EQUILIB_POTS
C###  Description:
C###    Calculates the equilibrium potentials for the ATR model

      IMPLICIT NONE
      
      !parameters
      REAL*8 RTF,Nai,Ki,Cai,Nac,Kc,Cac
      !common blocks
      REAL*8 EK,ENa,ECa
      COMMON /ATR_EP/ EK,ENa,ECa

      ! Equilibrium potentials
      EK = RTF*DLOG(Kc/Ki)
      ENa = RTF*DLOG(Nac/Nai)
      ECa = RTF/2.d0*DLOG(Cac/Cai)

      RETURN
      END


      REAL*8 FUNCTION BETA_DM(Lamda)

C#### Function: BETA_DM
C###  Type: REAL*8
C###  Description:
C###    Calculation of beta (eq 48).

      IMPLICIT NONE
!     Passed variables
      INTEGER Lamda
!     Common blocks
      INTEGER    L
      COMMON /L/ L
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8 f1(9),g1(9),g2(9),f_dash0(9),f_dash1(9)
      COMMON /PARAMS1/ f1,g1,g2,f_dash0,f_dash1
!     Local variables
      INTEGER m !,IFAIL,IW(500)
C      REAL*8 ABSERR,W(2000)
C      EXTERNAL Beta_Integrand

      m=Muscle_type
      L=Lamda

      IF(Integration.EQ.1) THEN      !Analytic integration
        BETA_DM=f1(m)/(L+2)
C *** Remove numerical integration for now (temp???)
c      ELSE IF(Integration.EQ.2) THEN !Numerical integration
c        IFAIL=1
c        CALL D01AMF(Beta_Integrand,0.D0,2,1.E-8,1.E-6,BETA_DM,ABSERR,
c     '    W,2000,IW,500,IFAIL)
c        IF(IFAIL.NE.0) THEN
c          WRITE(*,'('' IFAIL='',I1)') IFAIL
c        ENDIF
      ENDIF

      RETURN
      END


      SUBROUTINE CONCUR(CUR)

C#### Subroutine: CONCUR
C###  Description:
C###    Transfers Heart concentrations & currents to arrays CON & CUR
C###    ??? DPN - CURRENTS only ???

      IMPLICIT NONE
      INCLUDE 'cell/oxs001.cmn'
!     Parameter List
      REAL*8 CUR(*) !,CON(*)

!      write(*,'('' >>>call concur'')')

      CUR( 1)=IACH
      CUR( 2)=IBCA
      CUR( 3)=IBK
      CUR( 4)=IBNA
      CUR( 5)=ICA
      CUR( 6)=ICA2
      CUR( 7)=ICA3
      CUR( 8)=ICACA
      CUR( 9)=ICAK
      CUR(10)=ICANA
      CUR(11)=IFK
      CUR(12)=IFNA
      CUR(13)=IK
      CUR(14)=IK1
      CUR(15)=IMK
      CUR(16)=INA
      CUR(17)=INACA
      CUR(18)=INAK
      CUR(19)=IP
      CUR(20)=ITO

      RETURN
      END


      SUBROUTINE COUPLED_SYSTEM(MECH_IPARAM,MEM_IPARAM1,MEM_IPARAM2,
     '  MEM_LPARAM,MECH_RPARAM1,MECH_RPARAM2,MEM_RPARAM1,MEM_RPARAM2,
     '  MEM_RPARAM3,MODEL_DEFINITION,NT_DAT,Y_FULL,PCURRENT_SWITCHES,
     '  EXTENSION_RATIO_VALUES)

C#### Subroutine: COUPLED_SYSTEM
C###  Description:
C###    <html>
C###    The main routine called to solve coupled/non-coupled systems.
C###    Y_FULL is passed in containing the initial values for the 
C###    membrane model variables, and is returned containing the values
C###    for all model variables at each TABT between TSTART and TEND.
C###    MODEL_DEFINITION specifies the models to be used:
C###    <ul>
C###      <li>MODEL_DEFINITION(1) - Mechanics model
C###          <ul>
C###            <li> 0 - no mechanics model
C###            <li> 1 - distribution moment model (not implemented)
C###            <li> 2 - fading memory model
C###            <li> 3 - Hill model (not implemented)
C###            <li> 4 - HMT (tmp??)
C###          </ul>
C###      <li>MODEL_DEFINITION(2) - Membrane model
C###          <ul>
C###            <li> 0 - no membrane model
C###            <li> 1 - DiFrancesco-Noble model
C###            <li> 2 - Hodgkin-Huxley model (not implemented)
C###            <li> 3 - Luo-Rudy model (not coupled ?? (done??) )
C###            <li> 4 - Jafri-Rice-Winslow model
C###            <li> 5 - Noble98
C###          </ul>
C###      <li>MODEL_DEFINITION(3) - Metabolism model (not implemented)
C###      <li>MODEL_DEFINITION(4) - Text output switch
C###          <ul>
C###            <li>0 - no text output
C###            <li>1 - text output
C###          </ul>
C###      <li>MODEL_DEFINITION(5) - Testing switch (allows for Y_FULL
C###          to be written out to the screen)
C###          <ul>
C###            <li>0 - not testing
C###            <li>1 - testing
C###          </ul>
C###    </ul>
C###    </html>

C *** Passed variables:
C ***   > all MECH_* arrays - fading memory parameters
C ***   > MEM_IPARAM*,MEM_LPARAM,MEM_RPARAM1,MEM_RPARAM2 
C ***       - OXSOFT (DiFrancesco-Noble) parameters (some from 
C ***         MEM_RPARAM1 used with Luo-Rudy model)
C ***   > MEM_RPARAM3 - Luo-Rudy specific parameters (& 
C ***                   Jafri-Rice-Winslow
C ***   > MODEL_DEFINITION - see above
C ***   > NT_DAT - the number of samples from each solution variable
C ***   > Y_FULL - see above
C ***   > PCURRENT_SWITCHES - either 0 or 1 to specify whether a 
C ***                         specific should be included in the 
C ***                         total current.
C ***   > EXTENSION_RATIO_VALUES - value for extension ratio at each
C ***                              mech. time step.
C ***
C *** For a complete description see: (CardiacCellModel)/help/???.html
      
C *** NOTE:
C ***   Luo-Rudy model is in milliseconds
C ***   Fading memory and DiFrancesco-Noble in seconds
C ***   Noble98 also in seconds

      IMPLICIT NONE
      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/cell00.cmn'
      INCLUDE 'cell/oxs005.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/lr001.cmn'
!     Passed variables
      INTEGER MECH_IPARAM(3),MEM_IPARAM1(9),MEM_IPARAM2(13),
     '  MODEL_DEFINITION(5),NT_DAT,PCURRENT_SWITCHES(30)
      LOGICAL MEM_LPARAM(5)
      REAL*4  Y_FULL(*),EXTENSION_RATIO_VALUES(*)
      REAL*8  MECH_RPARAM1(52),MECH_RPARAM2(61),MEM_RPARAM1(21,9),
     '  MEM_RPARAM2(9),MEM_RPARAM3(70)
!     Common blocks
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
C *** Temp. should use one common block for both mech and mem.
      REAL*8          MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      COMMON /MECH_T/ MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      REAL*8        L
      COMMON /PROP/ L
      INTEGER          count
      REAL*8           T
      COMMON /TEMPFIX/ T,count
      REAL*8        mech_pCa,Ca_m,z,Cb,PHI(3),To_initial
      COMMON /VARS/ mech_pCa,Ca_m,z,Cb,PHI,To_initial
      INTEGER          NT_EQN,NT_CUR,NT_MECH,NO_FULL_MEM,NO_FULL_MECH,
     '                 NO_DAT,NUM_CUR
      COMMON /NUMBERS/ NT_EQN,NT_CUR,NT_MECH,NO_FULL_MEM,NO_FULL_MECH,
     '                 NO_DAT,NUM_CUR
      !Luo-Rudy common blocks
      REAL*8        CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
      COMMON /conc/ CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
      REAL*8        Cai_on,Cai2
      COMMON /Cai_/ Cai_on,Cai2
      REAL*8        I_Na,I_Ca,I_K,F_NSR,F_JSR
      COMMON /isum/ I_Na,I_Ca,I_K,F_NSR,F_JSR
      REAL*8        I(19),ICa !DPN 3/10/97 - extending I()
      COMMON /curr/ I,ICa
      REAL*8 JRW_INa,JRW_ICa,JRW_IK,JRW_IK1,JRW_IKp,JRW_INaCa,JRW_INaK,
     '  JRW_InsCa,JRW_IpCa,JRW_ICaK,JRW_ICab,JRW_INab,
     '  JRW_InsNa,JRW_InsK,Jrel,Jleak,Jup,Jtr,Jxfer,Jtrpn
      COMMON /JRW_I/ JRW_INa,JRW_ICa,JRW_IK,JRW_IK1,JRW_IKp,JRW_INaCa,
     '  JRW_INaK,JRW_InsCa,JRW_IpCa,JRW_ICaK,JRW_ICab,JRW_INab,
     '  JRW_InsNa,JRW_InsK,Jrel,Jleak,Jup,Jtr,Jxfer,Jtrpn
      REAL*8             Ca_from_membrane,Ca_new_from_membrane,
     '  LTRPNCa_from_JRW
      COMMON /STORED_CA/ Ca_from_membrane,Ca_new_from_membrane,
     '  LTRPNCa_from_JRW
      INTEGER                     current_time_step,previous_time_step
      COMMON /TIME_VARIABLE_INFO/ current_time_step,previous_time_step
      !NOBLE98 common blocks
      REAL*8 INa98,IpNa98,IbNa98,IK198,IKr98,IKs98,IbK98,IKATP98,
     '  IKNa98,IK98,ICaLCa98,ICaLK98,ICaLNa98,ICaLCads98,ICaLKds98,
     '  ICaLNads98,IbCa98,IKACH98,INaK98,INaCa98,INaCads98,Iup98,
     '  Itr98,Irel98,XSRrel98,Ito98
      COMMON /OH5_CURRENTS/
     '  INa98,IpNa98,IbNa98,IK198,IKr98,IKs98,IbK98,IKATP98,
     '  IKNa98,IK98,ICaLCa98,ICaLK98,ICaLNa98,ICaLCads98,ICaLKds98,
     '  ICaLNads98,IbCa98,IKACH98,INaK98,INaCa98,INaCads98,Iup98,
     '  Itr98,Irel98,XSRrel98,Ito98
      REAL*8 my_To,my_T,my_Cb,my_z,my_time,my_Q
      COMMON /MY_STUFF/ my_To,my_T,my_Cb,my_z,my_time,my_Q
!     Local variables
      LOGICAL TESTING
      INTEGER TEST_COUNT
      INTEGER NO_EQN,NO_CUR !,NT_EQN
      INTEGER loop_end,add_on,NT_EQN_DM
      INTEGER IFLAG
      REAL*8  current_time,t_beg,tfinal,Y_EQN(50),CUR(20),
     '  TNEXT,FINAL_TIME,TIME_STEP,Y_EQN_DM(5),jrw_current_time
!,TCURRENT,FN_TO   SMAR009 23/12/98 ,CON(20)

C *** Need this to get ionic currents for plotting
C *** Needs to be FALSE if paralellising !!
      SINGLE_CELL = .TRUE.

C *** initialise the Ca bound to low-affinity troponin
      LTRPNCa_from_JRW = -1.0d0

C *** Get the text output switch
      IF(MODEL_DEFINITION(4).EQ.1) THEN
        DEBUG = .TRUE.
      ELSE
        DEBUG = .FALSE.
      ENDIF
C *** Get the testing switch
      IF(MODEL_DEFINITION(5).EQ.1) THEN
        TESTING = .TRUE.
        TEST_COUNT = 0
      ELSE
        TESTING = .FALSE.
      ENDIF
      IF(DEBUG) write(*,'('' >>>call COUPLED_SYSTEM'')')
      IF(TESTING) OPEN(unit=9,file='testing.output',status='UNKNOWN')
C *** Get the model types
C *** NOTE: currently only works with DiFrancesco-Noble model and
C ***       Fading Memory model
C ***       DPN 26/11/97 - adding distribution moment model
      MECHANICS_MODEL = MODEL_DEFINITION(1)
      MEMBRANE_MODEL  = MODEL_DEFINITION(2)
      IF(MEMBRANE_MODEL.EQ.0.OR.MEMBRANE_MODEL.EQ.2) THEN
        NT_EQN  = 0
        NUM_CUR = 30
        IF(DEBUG) write(*,'('' >>>>Membrane model: None'')')
      ELSE IF(MEMBRANE_MODEL.EQ.1) THEN
C ***   Initialise DiFrancesco-Noble model
        IF(DEBUG) 
     '    write(*,'('' >>>>Membrane model: DiFrancesco - Noble'')')
        NT_EQN=33
        NUM_CUR=30
        NT_MECH=2        
C       Initialize
        CALL OXSINI(MEM_IPARAM1,MEM_IPARAM2,MEM_LPARAM,MEM_RPARAM1,
     '    MEM_RPARAM2,Y_EQN)
        CALL OXSPREP(Y_EQN)
        DO NO_EQN=1,NT_EQN
          Y_EQN(NO_EQN)=DBLE(Y_FULL(NO_EQN))
        ENDDO
C ***   Grab the initial [Ca]i
        Ca_from_membrane     = Y_EQN(4)*1.0d3
        Ca_new_from_membrane = Y_EQN(4)*1.0d3
        !set LTRPNCa_from_JRW negative, since not using JRW
        LTRPNCa_from_JRW = -1.0d0
        NO_FULL_MEM=NT_EQN !+NUM_CUR+NT_MECH
C       Write initial concentrations, currents and y's
c        CALL CONCUR(CON,CUR)
        CALL CONCUR(CUR)
        CALL LIOXSPARA(Y_EQN) !list parameters
        IF(MECHANICS_MODEL.EQ.2) THEN
          loop_end = NUM_CUR
          add_on = NT_MECH
        ELSE
          loop_end = NUM_CUR+NT_MECH
          add_on = 0
        ENDIF
C ***    Currents are already initialised to zero
c        DO NO_CUR=1,loop_end !NUM_CUR+NT_MECH
c          NO_FULL_MEM=NO_FULL_MEM+1
c          IF(NO_CUR.LE.20) THEN
c            Y_FULL(NO_FULL_MEM)=SNGL(CUR(NO_CUR))
c          ELSE
c            Y_FULL(NO_FULL_MEM) = 0.0e0
c          ENDIF
c        ENDDO
        NO_FULL_MEM = NO_FULL_MEM + NUM_CUR + NT_MECH !add_on
        !TCURRENT=TSTART
        IFLAG=0 !for 1st call to DESOL
        t_beg = TSTART
        FINAL_TIME = TEND
        TIME_STEP = TABT
C ***   assign the current switches
        DO NO_CUR=1,30
          ISWTCH(NO_CUR) = DBLE(PCURRENT_SWITCHES(NO_CUR))
        ENDDO
      ELSE IF(MEMBRANE_MODEL.EQ.3) THEN
C ***   Initialise Luo-Rudy model
        IF(DEBUG) write(*,'('' >>>>Membrane model: Luo - Rudy'')')
        NT_EQN=14
        NUM_CUR=30
        NT_MECH=2
        !used for placing currents into Y_FULL()
        IF(MECHANICS_MODEL.EQ.2) THEN
          loop_end = NUM_CUR
          add_on = NT_MECH
        ELSE
          loop_end = NUM_CUR+NT_MECH
          add_on = 0
        ENDIF
        !DPN 3/11/97 - initialise Cai_on and Cai2 each time this 
        !function is called. Required due to static variables ???
        Cai_on = 0.0d0
        Cai2 = 0.0d0
        !concentrations
        Ko  = MEM_RPARAM1( 1,3) !bulk external [K]
        Kb  = Ko !use to avoid common block in LR_CURRENTS()
        Ki  = MEM_RPARAM1( 2,3) ![K]i
        Nai = MEM_RPARAM1( 3,3) ![Na]i
        Nao = MEM_RPARAM1( 7,3) ![Na]o
        Cai = MEM_RPARAM1( 4,3) ![Ca]i
        Cao = MEM_RPARAM1( 5,3) ![Ca]o
        !Set parameters
        CALL DEFINE_LR(MEM_RPARAM3)
        !time steps (ms)
        TIMESCALE = MEM_RPARAM1( 1,9) 
        TSTART    = MEM_RPARAM1( 2,9) !inital value of t
        TEND      = MEM_RPARAM1( 3,9) !final value of t
        DT        = MEM_RPARAM1( 4,9) !time increment
        TABT      = MEM_RPARAM1( 5,9) !tabulation interval
        TPS       = MEM_RPARAM1( 6,9) !value of t when stimulus starts
        TP        = MEM_RPARAM1( 7,9) !duration of stimulus
        DO NO_EQN=1,NT_EQN
          Y_EQN(NO_EQN)=DBLE(Y_FULL(NO_EQN))
        ENDDO
C ***   Grab the initial [Ca]i
        Ca_from_membrane     = Y_EQN(3)*1.0d3
        Ca_new_from_membrane = Y_EQN(3)*1.0d3
        !set LTRPNCa_from_JRW negative, since not using JRW
        LTRPNCa_from_JRW = -1.0d0
C        NO_FULL_MEM = NO_FULL_MEM + NUM_CUR + NT_MECH !add_on
        NO_FULL_MEM = NT_EQN+NUM_CUR+NT_MECH
        !TCURRENT=TSTART
        IFLAG=0 !for 1st call to DESOL
        t_beg = TSTART*1.0d-3
        FINAL_TIME = TEND*1.0d-3
        TIME_STEP = TABT*1.0d-3
C ***   assign the current switches
        DO NO_CUR=1,30
          ISWTCH(NO_CUR) = DBLE(PCURRENT_SWITCHES(NO_CUR))
        ENDDO
      ELSE IF(MEMBRANE_MODEL.EQ.4) THEN
C ***   Initialise Jafri-Rice-Winslow model

C ***   DPN 23/07/98 - set the tension dependent relase switch
        IF (MEM_IPARAM2(1).NE.0) THEN
          TENSION_DEPENDENT_RELEASE = .TRUE.
          WRITE(*,*) 'Tension dependent release of Ca from Tn is on'
        ELSE
          TENSION_DEPENDENT_RELEASE = .FALSE.
          WRITE(*,*) 'Tension dependent release of Ca from Tn is off'
        ENDIF

        IF(DEBUG) 
     '    write(*,'('' >>>>Membrane model: Jafri - Rice - Winslow'')')
        NT_EQN=31
        NUM_CUR=30
        NT_MECH=2
        !used for placing currents into Y_FULL()
        IF(MECHANICS_MODEL.EQ.2) THEN
          loop_end = NUM_CUR
          add_on = NT_MECH
        ELSE
          loop_end = NUM_CUR+NT_MECH
          add_on = 0
        ENDIF
        !Set parameters
        CALL DEFINE_JRW(MEM_RPARAM3)
        WRITE(*,*) 'Stimulus frequency = ',STIM_FREQ
        !time steps (ms)
        TIMESCALE = MEM_RPARAM1( 1,9) 
        TSTART    = MEM_RPARAM1( 2,9) !inital value of t
        TEND      = MEM_RPARAM1( 3,9) !final value of t
        DT        = MEM_RPARAM1( 4,9) !time increment
        TABT      = MEM_RPARAM1( 5,9) !tabulation interval
        TPS       = MEM_RPARAM1( 6,9) !value of t when stimulus starts
        TP        = MEM_RPARAM1( 7,9) !duration of stimulus
c        write(*,*) 'Enter the frequency of stimulation'
c        read(*,*) STIM_FREQ
C DPN 16/7/98 - now included in CELL parameters for JRW only
c        STIM_FREQ = 0.0d0 !frequency of stimulation (Hz)
        DO NO_EQN=1,NT_EQN
          Y_EQN(NO_EQN)=DBLE(Y_FULL(NO_EQN))
        ENDDO
C ***   Convert to consistent units
c        Y_EQN( 3) = Y_EQN( 3)*1.0d-6 ![Ca]i nM -> mM
c        Y_EQN(31) = Y_EQN(31)*1.0d-3 !HTRPNCa uM -> mM
c        Y_EQN(30) = Y_EQN(30)*1.0d-3 !LTRPNCa uM -> mM
C ***   Grab the initial [Ca]i
        Ca_from_membrane     = Y_EQN(3)*1.0d3 ! mM -> uM
        Ca_new_from_membrane = Y_EQN(3)*1.0d3 ! mM -> uM
C ***   Grab the initial [Ca] bound to low-affinity troponin
        LTRPNCa_from_JRW = Y_EQN(30)*1.0d3    ! mM -> uM
C        NO_FULL_MEM = NO_FULL_MEM + NUM_CUR + NT_MECH !add_on
        NO_FULL_MEM = NT_EQN+NUM_CUR+NT_MECH
        !TCURRENT=TSTART
        IFLAG=0 !for 1st call to DESOL
        t_beg = TSTART
        FINAL_TIME = TEND
        TIME_STEP = TABT
C ***   assign the current switches
        DO NO_CUR=1,30
          ISWTCH(NO_CUR) = DBLE(PCURRENT_SWITCHES(NO_CUR))
        ENDDO
      ELSE IF(MEMBRANE_MODEL.EQ.5) THEN
C ***   Initialise Noble98 model
        IF(DEBUG) write(*,'('' >>>>Membrane model: Noble98'')')
C ***   DPN 02/12/98 - set the INa equations to use
        IF (MEM_IPARAM2(1).NE.0) THEN
          USE_LR_INA = .TRUE.
          WRITE(*,*) 'Using LR INa equations'
        ELSE
          USE_LR_INA = .TRUE.
          WRITE(*,*) 'Using Noble 98 INa equations'
        ENDIF
        NT_EQN=23
        NUM_CUR=30
        NT_MECH=2
        !used for placing currents into Y_FULL()
        IF(MECHANICS_MODEL.EQ.2) THEN
          loop_end = NUM_CUR
          add_on = NT_MECH
        ELSE
          loop_end = NUM_CUR+NT_MECH
          add_on = 0
        ENDIF
        !Set parameters
        CALL DEFINE_NOBLE98(MEM_RPARAM3)
        !time steps (ms)
        TIMESCALE = MEM_RPARAM1( 1,9) 
        TSTART    = MEM_RPARAM1( 2,9)*1.0d-3 !ms -> s !inital value of t
        TEND      = MEM_RPARAM1( 3,9)*1.0d-3 !final value of t
        DT        = MEM_RPARAM1( 4,9)*1.0d-3 !time increment
        TABT      = MEM_RPARAM1( 5,9)*1.0d-3 !tabulation interval
        TPS       = MEM_RPARAM1( 6,9)*1.0d-3 !value of t when stimulus starts
        TP        = MEM_RPARAM1( 7,9)*1.0d-3 !duration of stimulus
        DO NO_EQN=1,NT_EQN
          Y_EQN(NO_EQN)=DBLE(Y_FULL(NO_EQN))
        ENDDO
C ***   Grab the initial [Ca]i
c        Ca_from_membrane     = Y_EQN(3)*1.0d3
c        Ca_new_from_membrane = Y_EQN(3)*1.0d3
        !set LTRPNCa_from_JRW negative, since not using JRW
        LTRPNCa_from_JRW = -1.0d0
C        NO_FULL_MEM = NO_FULL_MEM + NUM_CUR + NT_MECH !add_on
        NO_FULL_MEM = NT_EQN+NUM_CUR+NT_MECH
        !TCURRENT=TSTART
        IFLAG=0 !for 1st call to DESOL
        t_beg = TSTART
        FINAL_TIME = TEND
        TIME_STEP = TABT
C ***   assign the current switches
        DO NO_CUR=1,30
          ISWTCH(NO_CUR) = DBLE(PCURRENT_SWITCHES(NO_CUR))
        ENDDO
      ENDIF !membrane model type
      IF(MECHANICS_MODEL.EQ.1) THEN
C ***   Initialise distribution moment model
        IF(DEBUG) 
     '    write(*,'('' >>>>Mechanics model: Distribution moment'')')
        !initialise fading memory parameters (temp???)
        CALL DISTRIBUTION_MOMENT(MECH_IPARAM,MECH_RPARAM1,MECH_RPARAM2,
     '    NT_DAT,Y_FULL,Y_EQN_DM,NT_EQN_DM,MEMBRANE_MODEL)
        IF(MEMBRANE_MODEL.EQ.0) THEN
          DO NO_EQN=1,NT_EQN+NUM_CUR
            Y_FULL(NO_EQN) = 0.0e0
          ENDDO
        ENDIF
        IFLAG=0 !for 1st call to DESOL
        t_beg = MECH_TSTART
        FINAL_TIME = MECH_TEND
        TIME_STEP = MECH_TABT
        DT = MECH_DT
      ELSE IF(MECHANICS_MODEL.EQ.2) THEN
C ***   Initialise the fading memory model
        IF(DEBUG) write(*,'('' >>>>Mechanics model: Fading Memory'')')
C ***   DPN 30 April 98 - initialise time variable info
        current_time_step = 1
        previous_time_step = 0
        CALL FADING_MEMORY(MECH_IPARAM,MECH_RPARAM1,MECH_RPARAM2,NT_DAT,
     '    Y_FULL,NT_EQN,t_beg,MEMBRANE_MODEL,LTRPNCa_from_JRW)
        IF(MEMBRANE_MODEL.EQ.0) THEN
          DO NO_EQN=1,NT_EQN+NUM_CUR
            Y_FULL(NO_EQN) = 0.0e0
          ENDDO
        ENDIF
        FINAL_TIME = MECH_TEND
        TIME_STEP = MECH_TABT
      ELSE IF(MECHANICS_MODEL.EQ.4) THEN
        NT_EQN = 2
        NO_FULL_MECH = 0
C ***   Initialise the HMT model
        IF(.TRUE.) write(*,'('' >>>>Mechanics model: HMT'')')
C ***   DPN 30 April 98 - initialise time variable info
        current_time_step = 1
        previous_time_step = 0
        CALL FADING_MEMORY(MECH_IPARAM,MECH_RPARAM1,MECH_RPARAM2,NT_DAT,
     '    Y_FULL,NT_EQN,t_beg,MEMBRANE_MODEL,LTRPNCa_from_JRW)
        NO_FULL_MECH = 0
c        Y_FULL(NO_FULL_MECH) = L
c        NO_FULL_MECH = NO_FULL_MECH + 1
c        Y_FULL(NO_FULL_MECH) = T
c        NO_FULL_MECH = NO_FULL_MECH + 1
c        Y_FULL(NO_FULL_MECH) = 0.0
c        NO_FULL_MECH = NO_FULL_MECH + 1
c        Y_FULL(NO_FULL_MECH) = 0.0
c        NO_FULL_MECH = NO_FULL_MECH + 1
c        Y_FULL(NO_FULL_MECH) = 0.0
c        NO_FULL_MECH = NO_FULL_MECH + 1
c        Y_FULL(NO_FULL_MECH) = 0.0
c        NO_FULL_MECH = NO_FULL_MECH + 1
c        Y_FULL(NO_FULL_MECH) = 0.0

c        IF(MEMBRANE_MODEL.EQ.0) THEN
c          DO NO_EQN=1,NT_EQN
c            Y_FULL(NO_EQN) = 0.0e0
c          ENDDO
c        ENDIF
        FINAL_TIME = MECH_TEND
        TIME_STEP = MECH_TABT
      ENDIF !mechanics model type
C *** Solve the model, output every TABT
C *** Need to add checks for no models to solve (if this should ever
C ***   get this far)
      DO current_time=t_beg,FINAL_TIME,TIME_STEP!MECH_TEND,MECH_TABT
        IF(MEMBRANE_MODEL.EQ.1) THEN
          !solve the Difrancesco-Noble model
          IF(IFLAG.LE.1) THEN
            TNEXT=current_time+TABT !TCURRENT+TABT
            CALL DFN_CHANGE(current_time,Y_EQN,IFLAG) !TCURRENT,Y_EQN)
            CALL DESOL('DN',NT_EQN,current_time,TNEXT,Y_EQN,IFLAG)!TCURRENT
            IF(IFLAG.LE.1) THEN
              IF(MECHANICS_MODEL.EQ.2) THEN
C ***           Grab the new [Ca]i, keeping the initial value
                Ca_from_membrane = Ca_new_from_membrane 
                Ca_new_from_membrane = Y_EQN(4)*1.0d3
              ENDIF
C             Write concentrations, currents and y's
c              CALL CONCUR(CON,CUR)
              CALL CONCUR(CUR)
              DO NO_EQN=1,NT_EQN
                NO_FULL_MEM=NO_FULL_MEM+1
                Y_FULL(NO_FULL_MEM)=SNGL(Y_EQN(NO_EQN))
                IF (TESTING) THEN
                  TEST_COUNT = TEST_COUNT + 1
                  IF (TEST_COUNT.GT.100) TEST_COUNT = 0
                  IF (TEST_COUNT.EQ.100) THEN
                    WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MEM)
                  ENDIF
                ENDIF
              ENDDO
              DO NO_CUR=1,loop_end !NUM_CUR+NT_MECH
                NO_FULL_MEM=NO_FULL_MEM+1
                IF(NO_CUR.LE.20) THEN
                  Y_FULL(NO_FULL_MEM)=SNGL(CUR(NO_CUR))
C *** DPN 10/08/98 - output total I(f) as I(nsCa)
                ELSEIF(NO_CUR.EQ.22) THEN
                  Y_FULL(NO_FULL_MEM)=SNGL(CUR(12)+CUR(11))!I(FNa)+I(fK)
                ELSE
                  Y_FULL(NO_FULL_MEM) = 0.0e0
                ENDIF
                IF (TESTING) THEN
                  TEST_COUNT = TEST_COUNT + 1
                  IF (TEST_COUNT.GT.100) TEST_COUNT = 0
                  IF (TEST_COUNT.EQ.100) THEN
                    WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MEM)
                  ENDIF
                ENDIF
              ENDDO
              NO_FULL_MEM = NO_FULL_MEM + add_on
              !NO_FULL_MEM=NO_FULL_MEM
              !TCURRENT=TNEXT
            ENDIF !IFLAG
          ELSE
            GOTO 99
          ENDIF !IFLAG
        ELSE IF(MEMBRANE_MODEL.EQ.3) THEN
          !solve the Luo-Rudy model
          IF(IFLAG.LE.1) THEN
C ***       If fading memory model set, need to convert times
c            IF(MECHANICS_MODEL.EQ.2) THEN
c              TNEXT=current_time*1.0d3+TABT !TCURRENT+TABT
c              CALL DESOL('LR',NT_EQN,current_time*1.0d3,TNEXT,Y_EQN)
c            ELSE
c              TNEXT=current_time+TABT !TCURRENT+TABT
c              CALL DESOL('LR',NT_EQN,current_time,TNEXT,Y_EQN)
c            ENDIF
            TNEXT=current_time*1.0d3+TABT !TCURRENT+TABT
            CALL DESOL('LR',NT_EQN,current_time*1.0d3,TNEXT,Y_EQN,IFLAG)
            IF(IFLAG.LE.1) THEN
              IF(MECHANICS_MODEL.EQ.2) THEN
C ***           Grab the new [Ca]i, keeping the initial value
                Ca_from_membrane = Ca_new_from_membrane 
                Ca_new_from_membrane = Y_EQN(3)*1.0d3
              ENDIF
              !Write concentrations, currents and y's
              DO NO_EQN=1,NT_EQN
                NO_FULL_MEM=NO_FULL_MEM+1
                Y_FULL(NO_FULL_MEM)=SNGL(Y_EQN(NO_EQN))
                IF (TESTING) THEN
                  TEST_COUNT = TEST_COUNT + 1
                  IF (TEST_COUNT.GT.100) TEST_COUNT = 0
                  IF (TEST_COUNT.EQ.100) THEN
                    WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MEM)
                  ENDIF
                ENDIF
              ENDDO
              DO NO_CUR=1,loop_end
                NO_FULL_MEM=NO_FULL_MEM+1
                !initially set all currents to zero
                Y_FULL(NO_FULL_MEM)=0.0e0 
              ENDDO
              ! now place in appropriate currents
              ! ??? DPN - should be in its own subroutine ???
              NO_FULL_MEM = NO_FULL_MEM + add_on - NT_MECH
              Y_FULL(NO_FULL_MEM-28) = SNGL(I(10))
              Y_FULL(NO_FULL_MEM-26) = SNGL(I(11))
              Y_FULL(NO_FULL_MEM-25) = SNGL(I(2))
              Y_FULL(NO_FULL_MEM-22) = SNGL(I(17))
              Y_FULL(NO_FULL_MEM-21) = SNGL(I(18))
              Y_FULL(NO_FULL_MEM-20) = SNGL(I(19))
              Y_FULL(NO_FULL_MEM-17) = SNGL(I(3))
              Y_FULL(NO_FULL_MEM-16) = SNGL(I(4))
              Y_FULL(NO_FULL_MEM-14) = SNGL(I(1))
              Y_FULL(NO_FULL_MEM-13) = SNGL(I(6))
              Y_FULL(NO_FULL_MEM-12) = SNGL(I(7))
              Y_FULL(NO_FULL_MEM-9)  = SNGL(I(5))
              Y_FULL(NO_FULL_MEM-8)  = SNGL(I(8))
              Y_FULL(NO_FULL_MEM-7)  = SNGL(I(9))
              Y_FULL(NO_FULL_MEM-6)  = SNGL(I(12))
              Y_FULL(NO_FULL_MEM-5)  = SNGL(I(13))
              Y_FULL(NO_FULL_MEM-4)  = SNGL(I(14))
              Y_FULL(NO_FULL_MEM-3)  = SNGL(I(15))
              Y_FULL(NO_FULL_MEM-2)  = SNGL(I(16))
              Y_FULL(NO_FULL_MEM-1)  = SNGL(I_Na)
              Y_FULL(NO_FULL_MEM)    = SNGL(I_Ca)
              !TCURRENT=TNEXT
              NO_FULL_MEM=NO_FULL_MEM+NT_MECH
            ENDIF !IFLAG
          ELSE
            GOTO 99
          ENDIF !IFLAG
        ELSE IF(MEMBRANE_MODEL.EQ.4) THEN
          !solve the Jafri-Rice-Winslow model
          IF(IFLAG.LE.1) THEN
C ***       If fading memory model set, need to convert times
            IF (MECHANICS_MODEL.EQ.2) THEN
              jrw_current_time = current_time*1.0d3 !s -> ms if 
                                                    ! fading memory
            ELSE
              jrw_current_time = current_time
            ENDIF
            TNEXT=jrw_current_time+TABT !TCURRENT+TABT
            CALL DESOL('JRW',NT_EQN,jrw_current_time,TNEXT,Y_EQN,IFLAG)
            IF(IFLAG.LE.1) THEN
              IF(MECHANICS_MODEL.EQ.2) THEN
C ***           Grab the new [Ca]i, keeping the initial value
                Ca_from_membrane = Ca_new_from_membrane 
                Ca_new_from_membrane = Y_EQN(3)*1.0d3
C ***           Grab the [Ca] bound to low-affinity troponin
                LTRPNCa_from_JRW = Y_EQN(30)*1.0d3 ! mM -> uM
              ENDIF
              !Write concentrations, currents and y's
              DO NO_EQN=1,NT_EQN
                NO_FULL_MEM=NO_FULL_MEM+1
                Y_FULL(NO_FULL_MEM)=SNGL(Y_EQN(NO_EQN))
                IF (TESTING) THEN
                  TEST_COUNT = TEST_COUNT + 1
                  IF (TEST_COUNT.GT.100) TEST_COUNT = 0
                  IF (TEST_COUNT.EQ.100) THEN
                    WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MEM)
                  ENDIF
                ENDIF
              ENDDO
              DO NO_CUR=1,loop_end
                NO_FULL_MEM=NO_FULL_MEM+1
                !initially set all currents to zero
                Y_FULL(NO_FULL_MEM)=0.0e0 
              ENDDO
              ! now set calculated currents
              NO_FULL_MEM = NO_FULL_MEM + add_on - NT_MECH
              Y_FULL(NO_FULL_MEM-28) = SNGL(JRW_ICab)
              Y_FULL(NO_FULL_MEM-26) = SNGL(JRW_INab)
              Y_FULL(NO_FULL_MEM-25) = SNGL(JRW_ICa+JRW_ICaK)
              Y_FULL(NO_FULL_MEM-22) = SNGL(JRW_ICa)
              Y_FULL(NO_FULL_MEM-21) = SNGL(JRW_ICaK)
              Y_FULL(NO_FULL_MEM-17) = SNGL(JRW_IK)
              Y_FULL(NO_FULL_MEM-16) = SNGL(JRW_IK1)
              Y_FULL(NO_FULL_MEM-14) = SNGL(JRW_INa)
              Y_FULL(NO_FULL_MEM-13) = SNGL(JRW_INaCa)
              Y_FULL(NO_FULL_MEM-12) = SNGL(JRW_INaK)
              Y_FULL(NO_FULL_MEM- 9) = SNGL(JRW_IKp)
              Y_FULL(NO_FULL_MEM- 8) = SNGL(JRW_InsCa)
              Y_FULL(NO_FULL_MEM- 7) = SNGL(JRW_IpCa)
              Y_FULL(NO_FULL_MEM- 5) = SNGL(Jrel)
              Y_FULL(NO_FULL_MEM- 4) = SNGL(Jup)
              Y_FULL(NO_FULL_MEM- 3) = SNGL(Jleak)
              Y_FULL(NO_FULL_MEM- 2) = SNGL(Jtr)
              Y_FULL(NO_FULL_MEM-29) = SNGL(Jxfer)
              !Y_FULL(NO_FULL_MEM+ 1) = SNGL(Jtrpn)
              !TCURRENT=TNEXT
              NO_FULL_MEM=NO_FULL_MEM+NT_MECH
            ENDIF !IFLAG
          ELSE
            GOTO 99
          ENDIF !IFLAG
        ELSE IF(MEMBRANE_MODEL.EQ.5) THEN
          !solve the Noble98 model
          IF(IFLAG.LE.1) THEN
C ***       If fading memory model set, need to convert times
c            IF(MECHANICS_MODEL.EQ.2) THEN
c              TNEXT=current_time*1.0d3+TABT !TCURRENT+TABT
c              CALL DESOL('LR',NT_EQN,current_time*1.0d3,TNEXT,Y_EQN)
c            ELSE
c              TNEXT=current_time+TABT !TCURRENT+TABT
c              CALL DESOL('LR',NT_EQN,current_time,TNEXT,Y_EQN)
c            ENDIF
            TNEXT=current_time+TABT !TCURRENT+TABT
            CALL DESOL('N98',NT_EQN,current_time,TNEXT,Y_EQN,IFLAG)
            IF(IFLAG.LE.1) THEN
              IF(MECHANICS_MODEL.EQ.2) THEN
C ***           Grab the new [Ca]i, keeping the initial value
                Ca_from_membrane = Ca_new_from_membrane 
                Ca_new_from_membrane = Y_EQN(3)*1.0d3
              ENDIF
              !Write concentrations, currents and y's
              DO NO_EQN=1,NT_EQN
                NO_FULL_MEM=NO_FULL_MEM+1
                Y_FULL(NO_FULL_MEM)=SNGL(Y_EQN(NO_EQN))
                IF (TESTING) THEN
                  TEST_COUNT = TEST_COUNT + 1
                  IF (TEST_COUNT.GT.100) TEST_COUNT = 0
                  IF (TEST_COUNT.EQ.100) THEN
                    WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MEM)
                  ENDIF
                ENDIF
              ENDDO
              DO NO_CUR=1,loop_end
                NO_FULL_MEM=NO_FULL_MEM+1
                !initially set all currents to zero
                Y_FULL(NO_FULL_MEM)=0.0e0 
              ENDDO
              ! now place in appropriate currents
              ! ??? DPN - should be in its own subroutine ???
              NO_FULL_MEM = NO_FULL_MEM + add_on - NT_MECH
              Y_FULL(NO_FULL_MEM-14) = SNGL(INa98)
              Y_FULL(NO_FULL_MEM-7) = SNGL(IpNa98)
              Y_FULL(NO_FULL_MEM-26) = SNGL(IbNa98)
              Y_FULL(NO_FULL_MEM-16) = SNGL(IK198)
              Y_FULL(NO_FULL_MEM-19) = SNGL(IKr98)
              Y_FULL(NO_FULL_MEM-18) = SNGL(IKs98)
              Y_FULL(NO_FULL_MEM-27) = SNGL(IbK98)
              Y_FULL(NO_FULL_MEM-15) = SNGL(IKATP98)
              Y_FULL(NO_FULL_MEM-10) = SNGL(Ito98)
              Y_FULL(NO_FULL_MEM-9) = SNGL(IKNa98)
              Y_FULL(NO_FULL_MEM-17) = SNGL(IK98)
              Y_FULL(NO_FULL_MEM-22) = SNGL(ICaLCa98)
              Y_FULL(NO_FULL_MEM-21) = SNGL(ICaLK98)
              Y_FULL(NO_FULL_MEM-20) = SNGL(ICaLNa98)
              Y_FULL(NO_FULL_MEM-25) = SNGL(ICaLCads98)
              Y_FULL(NO_FULL_MEM-24) = SNGL(ICaLKds98)
              Y_FULL(NO_FULL_MEM-23) = SNGL(ICaLNads98)
              Y_FULL(NO_FULL_MEM-28) = SNGL(IbCa98)
              Y_FULL(NO_FULL_MEM-29) = SNGL(IKACh98)
              Y_FULL(NO_FULL_MEM-12) = SNGL(INaK98)
              Y_FULL(NO_FULL_MEM-13)  = SNGL(INaCa98)
              Y_FULL(NO_FULL_MEM-11)  = SNGL(INaCads98)
              Y_FULL(NO_FULL_MEM-4)  = SNGL(Iup98)
              Y_FULL(NO_FULL_MEM-2)  = SNGL(Itr98)
              Y_FULL(NO_FULL_MEM-5)  = SNGL(Irel98)
              Y_FULL(NO_FULL_MEM-8)  = SNGL(XSRrel98)
              !TCURRENT=TNEXT
              NO_FULL_MEM=NO_FULL_MEM+NT_MECH
            ENDIF !IFLAG
          ELSE
            GOTO 99
          ENDIF !IFLAG
        ENDIF !MEMBRANE_MODEL
        IF(MECHANICS_MODEL.EQ.1) THEN
          !solve the distribution moment model
          IF(IFLAG.LE.1) THEN
            TNEXT=current_time+MECH_TABT !TCURRENT+TABT
            CALL DESOL('DM',NT_EQN_DM,current_time,TNEXT,Y_EQN_DM,IFLAG)
            IF(IFLAG.LE.1) THEN
C             Write out L and T
              NO_FULL_MECH = NO_FULL_MECH + NT_EQN + NT_CUR + NT_MECH
              Y_FULL(NO_FULL_MECH-1) = SNGL(Y_EQN_DM(5))!extension ratio
              Y_FULL(NO_FULL_MECH) = SNGL(Y_EQN_DM(3))  !tension
              IF (TESTING) THEN
                WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MECH-1)
                WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MECH)
              ENDIF
              IF(MEMBRANE_MODEL.EQ.0) THEN
                DO NO_EQN=1,NT_EQN+NUM_CUR
                  Y_FULL(NO_FULL_MECH-NT_MECH-(NT_EQN+NUM_CUR-NO_EQN)) 
     '              = 0.0e0
                ENDDO
              ENDIF              
            ENDIF !IFLAG
          ELSE
            GOTO 99
          ENDIF !IFLAG
        ELSE IF(MECHANICS_MODEL.EQ.2) THEN
C ***     Solve the Fading memory model
          IF(current_time.EQ.t_beg) THEN
            CALL FM_SOLVE(current_time,current_time+MECH_TABT,count,T,
     '        tfinal,EXTENSION_RATIO_VALUES)
            !assign new tension and extension ratio to results
            NO_FULL_MECH = NO_FULL_MECH + NT_EQN + NT_CUR + NT_MECH
c            Y_FULL(NO_FULL_MECH-1) = SNGL(Cb)! temp output Cb
            Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
            Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
C ***       DPN 29/09/98 - could be useful ?
c            y_full(no_full_mech-2) = sngl(my_Cb)
c            y_full(no_full_mech-3) = sngl(my_z)
c            y_full(no_full_mech-4) = sngl(my_To)
c            y_full(no_full_mech-5) = sngl(my_Q)
            IF (TESTING) THEN
              WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MECH-1)
              WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MECH)
            ENDIF
            IF(MEMBRANE_MODEL.EQ.0) THEN
              DO NO_EQN=1,NT_EQN+NUM_CUR
                Y_FULL(NO_FULL_MECH-NT_MECH-(NT_EQN+NUM_CUR-NO_EQN)) = 
     '            0.0e0
              ENDDO
            ENDIF
          ELSE
            CALL FM_SOLVE(tfinal,current_time+MECH_TABT,count,T,tfinal,
     '        EXTENSION_RATIO_VALUES)
            !assign new tension and extension ratio to results
            NO_FULL_MECH = NO_FULL_MECH + NT_EQN + NT_CUR + NT_MECH
c            Y_FULL(NO_FULL_MECH-1) = SNGL(Cb) ! temp output Cb
            Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
            Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
C ***       DPN 29/09/98 - could be useful ?
c            y_full(no_full_mech-2) = sngl(my_Cb)
c            y_full(no_full_mech-3) = sngl(my_z)
c            y_full(no_full_mech-4) = sngl(my_To)
            IF (TESTING) THEN
              WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MECH-1)
              WRITE(9,'(F15.5)') Y_FULL(NO_FULL_MECH)
            ENDIF
            IF(MEMBRANE_MODEL.EQ.0) THEN
              DO NO_EQN=1,NT_EQN+NUM_CUR
                Y_FULL(NO_FULL_MECH-NT_MECH-(NT_EQN+NUM_CUR-NO_EQN)) = 
     '            0.0e0
              ENDDO
            ENDIF
          ENDIF
        ELSE IF(MECHANICS_MODEL.EQ.4) THEN
C ***     Solve the HMT model
          IF(current_time.EQ.t_beg) THEN
            CALL FM_SOLVE(current_time,current_time+MECH_TABT,count,T,
     '        tfinal,EXTENSION_RATIO_VALUES)
            !assign new tension and extension ratio to results
            NO_FULL_MECH = NO_FULL_MECH + NT_EQN
c            Y_FULL(NO_FULL_MECH-1) = SNGL(Cb)! temp output Cb
            Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
            Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
C ***       DPN 29/09/98 - could be useful ?
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_T)
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_Cb)
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_z)
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_To)
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_Q)
          ELSE
            CALL FM_SOLVE(tfinal,current_time+MECH_TABT,count,T,tfinal,
     '        EXTENSION_RATIO_VALUES)
            !assign new tension and extension ratio to results
            NO_FULL_MECH = NO_FULL_MECH + NT_EQN
c            Y_FULL(NO_FULL_MECH-1) = SNGL(Cb) ! temp output Cb
            Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
            Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
C ***       DPN 29/09/98 - could be useful ?
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_T)
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_Cb)
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_z)
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_To)
            NO_FULL_MECH = NO_FULL_MECH + 1
            y_full(no_full_mech) = sngl(my_Q)
          ENDIF
        ENDIF !MECHANICS_MODEL
      ENDDO !current_time
 99   IF(MEMBRANE_MODEL.EQ.1.AND.IFLAG.LE.1.AND.
     '  MECHANICS_MODEL.EQ.2.AND.Test_type.NE.5.AND.
     '  Test_type.LT.20) THEN
C ***   put in the final values for the DiFrancesco-Noble model
        DO NO_EQN=1,NT_EQN
          NO_FULL_MEM=NO_FULL_MEM+1
          Y_FULL(NO_FULL_MEM)=SNGL(Y_EQN(NO_EQN))
        ENDDO
        DO NO_CUR=1,loop_end !NUM_CUR+NT_MECH
          NO_FULL_MEM=NO_FULL_MEM+1
          IF(NO_CUR.LE.20) THEN
            Y_FULL(NO_FULL_MEM)=SNGL(CUR(NO_CUR))
          ELSE
            Y_FULL(NO_FULL_MEM) = 0.0e0
          ENDIF
        ENDDO
      ELSE IF(MEMBRANE_MODEL.EQ.3.AND.IFLAG.LE.1.AND.
     '  MECHANICS_MODEL.EQ.2.AND.Test_type.NE.5.AND.
     '  Test_type.LT.20) THEN
C ***   put in the final values for the Luo-Rudy model
        DO NO_EQN=1,NT_EQN
          NO_FULL_MEM=NO_FULL_MEM+1
          Y_FULL(NO_FULL_MEM)=SNGL(Y_EQN(NO_EQN))
        ENDDO
        DO NO_CUR=1,loop_end
          NO_FULL_MEM=NO_FULL_MEM+1
          !initially set all currents to zero
          Y_FULL(NO_FULL_MEM)=0.0e0 
        ENDDO
        !now place in appropriate currents
        NO_FULL_MEM = NO_FULL_MEM + add_on - NT_MECH
        Y_FULL(NO_FULL_MEM-28) = SNGL(I(10))
        Y_FULL(NO_FULL_MEM-26) = SNGL(I(11))
        Y_FULL(NO_FULL_MEM-25) = SNGL(I(2))
        Y_FULL(NO_FULL_MEM-22) = SNGL(I(17))
        Y_FULL(NO_FULL_MEM-21) = SNGL(I(18))
        Y_FULL(NO_FULL_MEM-20) = SNGL(I(19))
        Y_FULL(NO_FULL_MEM-17) = SNGL(I(3))
        Y_FULL(NO_FULL_MEM-16) = SNGL(I(4))
        Y_FULL(NO_FULL_MEM-14) = SNGL(I(1))
        Y_FULL(NO_FULL_MEM-13) = SNGL(I(6))
        Y_FULL(NO_FULL_MEM-12) = SNGL(I(7))
        Y_FULL(NO_FULL_MEM-9)  = SNGL(I(5))
        Y_FULL(NO_FULL_MEM-8)  = SNGL(I(8))
        Y_FULL(NO_FULL_MEM-7)  = SNGL(I(9))
        Y_FULL(NO_FULL_MEM-6)  = SNGL(I(12))
        Y_FULL(NO_FULL_MEM-5)  = SNGL(I(13))
        Y_FULL(NO_FULL_MEM-4)  = SNGL(I(14))
        Y_FULL(NO_FULL_MEM-3)  = SNGL(I(15))
        Y_FULL(NO_FULL_MEM-2)  = SNGL(I(16))
        Y_FULL(NO_FULL_MEM-1)  = SNGL(I_Na)
        Y_FULL(NO_FULL_MEM)    = SNGL(I_Ca)
      ENDIF
      IF(MECHANICS_MODEL.EQ.2) THEN
C ***   put in the final values for the fading memory model
C DPN 18/03/98 - not required
c        IF(Test_type.EQ.8.OR.Test_type.EQ.9) THEN
c          NO_FULL_MECH = NO_FULL_MECH + NT_EQN + NT_CUR + NT_MECH
c          Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
c          Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
c        ELSE IF(Test_type.EQ.5) THEN
cc          NO_FULL_MECH = NO_FULL_MECH + NT_EQN + NT_CUR + NT_MECH
cc          Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
cc          Y_FULL(NO_FULL_MECH) = SNGL(FN_TO(L,z)) ! tension
c        ELSE IF(Test_type.EQ.20) THEN
cc          NO_FULL_MECH = NO_FULL_MECH + NT_EQN + NT_CUR + NT_MECH
cc          Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
cc          Y_FULL(NO_FULL_MECH) = SNGL(FN_TO(L,z)) ! tension
c        ELSE IF(Test_type.EQ.21) THEN
cc          NO_FULL_MECH = NO_FULL_MECH + NT_EQN + NT_CUR + NT_MECH
cc          Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
cc          Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
c        ELSE IF(Test_type.EQ.14) THEN
c          NO_FULL_MECH = NO_FULL_MECH + NT_EQN + NT_CUR + NT_MECH
c          Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
c          Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
c        ENDIF !Test_type
c        IF(MEMBRANE_MODEL.EQ.0) THEN
c          DO NO_EQN=1,NT_EQN+NUM_CUR
c            Y_FULL(NO_FULL_MECH-NT_MECH-(NT_EQN+NUM_CUR-NO_EQN)) = 0.0e0
c          ENDDO
c        ENDIF
      ELSEIF (MECHANICS_MODEL.EQ.4) THEN
        NO_FULL_MECH = NO_FULL_MECH + NT_EQN
        Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
        Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
C ***       DPN 29/09/98 - could be useful ?
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_T)
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_Cb)
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_z)
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_To)
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_Q)        
        NO_FULL_MECH = NO_FULL_MECH + NT_EQN
        Y_FULL(NO_FULL_MECH-1) = SNGL(L) ! extension ratio
        Y_FULL(NO_FULL_MECH) = SNGL(T) ! tension
C ***       DPN 29/09/98 - could be useful ?
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_T)
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_Cb)
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_z)
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_To)
        NO_FULL_MECH = NO_FULL_MECH + 1
        y_full(no_full_mech) = sngl(my_Q)        
      ENDIF !MECHANICS_MODEL

      IF(TESTING) CLOSE(unit=9)
      
      RETURN
      END


      SUBROUTINE DEFINE_ATR(RPARAM,Y)
     
C#### Subroutine: DEFINE_ATR
C###  Description:
C###    Initialises the ATR variables from those passed
C###    in from CELL, converting to a consistent set of units.

      IMPLICIT NONE
      
      INCLUDE 'cell/cell01.cmn'
      INCLUDE 'cell/deoxs00.cmn'
  
      REAL*8 RPARAM(70),Y(29)

      REAL*8           RTF
      COMMON /ATR_RTF/ RTF

      Y( 1) = -74.2525d0   !membrane potential
      Y( 2) = 3.2017d-3    !Na channel m gate
      Y( 3) = 0.8814d0     !Na channel h1 gate
      Y( 4) = 0.8742d0     !Na channel h2 gate
      Y( 5) = 1.3005d-5    !CaL Activation gating variable
      Y( 6) = 0.9986d0     !CaL Fast Inactivation gating variable
      Y( 7) = 0.9986d0     !CaL Slow Inactivation gating variable
      Y( 8) = 1.0678d-3    !K Transient Activation gating variable
      Y( 9) = 0.9490d0     !K Transient Inactivation gating variable
      Y(10) = 1.5949d-4    !K Sustained Activation gating variable
      Y(11) = 0.9912d0     !K Sustained Inactivation gating variable
      Y(12) = 4.8357d-3    !Ks delayed rectifier Activation gate 
      Y(13) = 0.0001d0     !Kr delayed rectifier Activation gate 
      Y(14) = 8.5547d0     !Intracellular [Na]
      Y(15) = 129.4350d0   !Intracellular [K]
      Y(16) = 6.7290d0     !Intracellular [Ca]
      Y(17) = 7.2495d0     !Restricted subsarcolemmal [Ca]
      Y(18) = 0.0275d0     !Ca Fractional occupancy in calmodulin buffer
      Y(19) = 0.0133d0     !Ca Fractional occupancy in troponin-Ca 
                           !buffer
      Y(20) = 0.1961d0     !Ca Fractional occupancy in troponin-Mg 
                           !buffer
      Y(21) = 0.7094d0     !Mg Fractional occupancy in troponin-Mg 
                           !buffer
      Y(22) = 0.4369d0     !Ca Fractional occupancy in calsequestrin 
                           !buffer
      Y(23) = 130.0110d0   !Extracellular Cleft space Na Concentrations
      Y(24) = 5.3581d0     !Extracellular Cleft space K Concentrations
      Y(25) = 1.8147d0     !Extracellular Cleft space Ca Concentrations
      Y(26) = 0.6465d0     !SR Release Compartment Ca concentration
      Y(27) = 0.6646d0     !SR Uptake Compartment Ca concentrations
      Y(28) = 0.4284d0     !Relative inactive precurser in Irel 
                           !formulation
      Y(29) = 0.0028d0     !Relative activator in Irel formulation

      !parameter values
      ATR_Nab = 130.0d0 !RPARAM(1)
      ATR_Kb  = 5.4d0 !RPARAM(2)
      ATR_Cab = 1.8d0 !RPARAM(3)
      ATR_Mgi = 2.5d0 !RPARAM(4)
      ATR_ECaapp = 60.0d0 !RPARAM(5)
      ATR_KCa = 0.25d0 !RPARAM(6)
      ATR_R = 8314.0d0 !RPARAM(7)
      ATR_Temp = 306.15d0 !RPARAM(8)
      ATR_Faraday = 96487.0d0 !RPARAM(9)
      ATR_Cm = 0.05d0 !RPARAM(10)
      ATR_Voli = 0.005884d0 !RPARAM(12)
      ATR_Volc = 0.136d0 * ATR_Voli !RPARAM(11)
      ATR_Vold = 0.02d0 * ATR_Voli !RPARAM(13)
      ATR_VolREL = 0.0000441d0 !RPARAM(14)
      ATR_VolUP = 0.0003969d0 !RPARAM(15)
      ATR_tau_Na = 14.3d0 !RPARAM(16)
      ATR_tau_K = 10.0d0 !RPARAM(17)
      ATR_tau_Ca = 24.7d0 !RPARAM(18)    
      ATR_tau_di = 0.010d0 !RPARAM(19)      
      ATR_INaK_max = 70.8253d0 !RPARAM(20)     
      ATR_KNaKK = 1.0d0 !RPARAM(21)   
      ATR_KNaKNa = 11.0d0 !RPARAM(22)
      ATR_ICaP_max = 4.0d0 !RPARAM(23)         
      ATR_KCaP = 0.0002d0 !RPARAM(24)
      ATR_KNaCa = 0.0374842d0 !RPARAM(25)     
      ATR_gamma = 0.45d0 !RPARAM(26)     
      ATR_dNaCa = 0.0003d0 !RPARAM(27)
      ATR_QNaen = -1.68d0 !RPARAM(28)       
      ATR_IUP_max = 2800.0d0 !RPARAM(29)
      ATR_Kcyca = 0.0003d0 !RPARAM(30)      
      ATR_Ksrca = 0.5d0 !RPARAM(31)
      ATR_Kxcs = 0.4d0 !RPARAM(32)
      ATR_tau_tr = 0.01d0 !RPARAM(33)
      ATR_alphaREL = 200000.0d0 !RPARAM(34)
      ATR_KRELi = 0.0003d0 !RPARAM(35)
      ATR_KRELd = 0.003d0 !RPARAM(36)
      ATR_rrecov = 0.815d0 !RPARAM(37)
      !Maximum conductance values
      ATR_PNa = 0.0016d0 !RPARAM(38)     
      ATR_GCaL = 6.75d0 !RPARAM(39)
      ATR_Gt = 7.5d0 !RPARAM(40)
      ATR_Gsus = 2.75d0 !RPARAM(41)
      ATR_GKs = 1.0d0 !RPARAM(42)     
      ATR_GKr = 0.5d0 !RPARAM(43)    
      ATR_GK1 = 3.0d0 !RPARAM(44)     
      ATR_GBNa = 0.060599d0 !RPARAM(45) 
      ATR_GBCa = 0.078681d0 !RPARAM(46) 
      !ATR_kp_htrpn = RPARAM(47)  (Not needed)
      !ATR_km_htrpn = RPARAM(48)  
      !ATR_kp_ltrpn = RPARAM(49)  
      !ATR_km_ltrpn = RPARAM(50)  
      !ATR_CMDN_tot = RPARAM(51)  
      !ATR_km_CMDN = RPARAM(52)   
      !ATR_GKp_max = RPARAM(53)
      !ATR_PNaK = RPARAM(54)
      !ATR_Nao = RPARAM(55)
      !ATR_Ko = RPARAM(56)
      !ATR_INaK_max = RPARAM(57)
      !ATR_km_Nai = RPARAM(58)
      !ATR_km_Ko = RPARAM(59)
      !ATR_GNab_max = RPARAM(60)
      ATR_Istim = 300.0d0!RPARAM(62)
      STIM_FREQ = 0.0d0!RPARAM(63) !??? 
      
      RTF  = ATR_R * ATR_Temp /ATR_Faraday
      
      RETURN
      END


      SUBROUTINE DEFINE_JRW(RPARAM)

C#### Subroutine: DEFINE_JRW
C###  Description:
C###    Initialises the Jafri-Rice-Winslow variables from those passed
C###    in from CELL, converting to a consistent set of units.

      IMPLICIT NONE

c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/cell00.cmn'
      INCLUDE 'cell/deoxs00.cmn'

      REAL*8 RPARAM(70)

      REAL*8           RTF
      COMMON /JRW_RTF/ RTF

c      integer i

      JRW_GCab_max = RPARAM(1)
      JRW_f = RPARAM(2)
      JRW_g = RPARAM(3)
      JRW_f_dash = RPARAM(4)
      JRW_g_dash = RPARAM(5)
      JRW_a = RPARAM(6)
      JRW_b = RPARAM(7)
      JRW_w = RPARAM(8)
      JRW_PCa = RPARAM(9)         *1.0d-3 !cm/s -> cm/(ms)
      JRW_PK = RPARAM(10)         *1.0d-3 !cm/s -> cm/(ms)
      JRW_ICa_half = RPARAM(11)
      JRW_kNaCa = RPARAM(12)
      JRW_km_Na = RPARAM(13)
      JRW_km_Ca = RPARAM(14)
      JRW_ksat = RPARAM(15)
      JRW_eta = RPARAM(16)
      JRW_ICap_max = RPARAM(17)
      JRW_km_Cap = RPARAM(18)     *1.0d-3 !uM -> mM
      JRW_P_ns = RPARAM(19)       *1.0d-3 !cm/s -> cm/(ms)
      JRW_km_ns = RPARAM(20)      *1.0d-3 !uM -> mM
      JRW_Faraday = RPARAM(21)    *1.0d6 !coul/mmol ((A.s)/mmol) -> (uA.ms)/umol
      JRW_Temp = RPARAM(22)
      JRW_R = RPARAM(23)          *1.0d-3 !J/mol/degK -> J/mmol/degK
      JRW_Acap = RPARAM(24)
      JRW_V_myo = RPARAM(25)      *1.0d-3 !uL -> cm*cm*cm
      JRW_V_SS = RPARAM(26)       *1.0d-3 !uL -> cm*cm*cm
      JRW_v1 = RPARAM(27)
      JRW_kap = RPARAM(28)        *1.0d3**4 !(uM)-4(ms)-1 ->(mM)-4(ms)-1
      JRW_kam = RPARAM(29)
      JRW_kbp = RPARAM(30)        *1.0d3**3 !(uM)-3(ms)-1 ->(mM)-3(ms)-1
      JRW_kbm = RPARAM(31)
      JRW_kcp = RPARAM(32)
      JRW_kcm = RPARAM(33)
      JRW_n = RPARAM(34)
      JRW_m = RPARAM(35)
      JRW_CSQN_tot = RPARAM(36)
      JRW_km_CSQN = RPARAM(37)
      JRW_V_JSR = RPARAM(38)      *1.0d-3 !uL -> cm*cm*cm
      JRW_tau_xfer = RPARAM(39)
      JRW_tau_tr = RPARAM(40)
      JRW_v2 = RPARAM(41)
      JRW_v3 = RPARAM(42)         *1.0d-3 !uM/(ms) -> mM/(ms)
      JRW_km_up = RPARAM(43)      *1.0d-3 !uM -> mM
      JRW_V_NSR = RPARAM(44)      *1.0d-3 !uL -> cm*cm*cm
      JRW_LTRPN_tot = RPARAM(45)  *1.0d-3 !uM -> mM
      JRW_HTRPN_tot = RPARAM(46)  *1.0d-3 !uM -> mM
      JRW_kp_htrpn = RPARAM(47)   *1.0d-6 !/M/s -> /mM/ms
      JRW_km_htrpn = RPARAM(48)   *1.0d-3 !/s -> /(ms)
      JRW_kp_ltrpn = RPARAM(49)   *1.0d-6 !/M/s -> /mM/ms
      JRW_km_ltrpn = RPARAM(50)   *1.0d-3 !/s -> /(ms)
      JRW_CMDN_tot = RPARAM(51)   *1.0d-3 !uM -> mM
      JRW_km_CMDN = RPARAM(52)    *1.0d-3 !uM -> mM
      JRW_GKp_max = RPARAM(53)
      JRW_PNaK = RPARAM(54)
      JRW_Nao = RPARAM(55)
      JRW_Ko = RPARAM(56)
      JRW_INaK_max = RPARAM(57)
      JRW_km_Nai = RPARAM(58)
      JRW_km_Ko = RPARAM(59)
      JRW_GNab_max = RPARAM(60)
      JRW_GNa_max = RPARAM(61)
      JRW_Istim = RPARAM(62)
      STIM_FREQ = RPARAM(63)

      RTF  = JRW_R * JRW_Temp / (JRW_Faraday*1.0d-6) ! Volts
      RTF  = RTF * 1.0d3                             ! mV 

c      do i=1,62
c        write(*,*) i,RPARAM(i)
c      enddo

      RETURN
      END


      SUBROUTINE DEFINE_LR(RPARAM)

C#### Subroutine: DEFINE_LR
C###  Description:
C###    Defines the Luo-Rudy parameters

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/lr001.cmn'
! Passed variables
      REAL*8 RPARAM(70)

      GGCab = RPARAM(1) 
      KmCa_L = RPARAM(2)
      PCa = RPARAM(3)
      gCao = RPARAM(4)
      kNaCa = RPARAM(5)
      KmNa = RPARAM(6)
      KmCa_NaCa = RPARAM(7)
      eta = RPARAM(8)
      ksat = RPARAM(9)
      KmpCa = RPARAM(10)
      IIpCa = RPARAM(11)
      KmnsCa = RPARAM(12)
      PnsCa = RPARAM(13)
      Cm = RPARAM(14)
      Acap = RPARAM(15)
      Temp = RPARAM(16)
      VolMyo = RPARAM(17)
      CSQNCSQN = RPARAM(18)
      KmCSQN = RPARAM(19)
      Kmrel = RPARAM(20)
      Tau_on = RPARAM(21)
      Tau_off = RPARAM(22)
      Caith = RPARAM(23)
      GGrel_ = RPARAM(24)
      VolJSR = RPARAM(25)
      Tau_tr = RPARAM(26)
      Kmup = RPARAM(27)
      CaNSRCaNSR = RPARAM(28)
      IIup = RPARAM(29)
      VolNSR = RPARAM(30)
      gCai = RPARAM(31)
      TRPNTRPN = RPARAM(32)
      KmTRPN = RPARAM(33)
      CMDNCMDN = RPARAM(34)
      KmCMDN = RPARAM(35) 
      GGKp = RPARAM(36)
      PK = RPARAM(37)
      gNai = RPARAM(38)
      gKi = RPARAM(39)
      gNao = RPARAM(40)
      gKo = RPARAM(41)
      PNaK = RPARAM(42)
      IINaK = RPARAM(43)
      KmNai = RPARAM(44)
      KmKo = RPARAM(45)
      GGNab = RPARAM(46)
      GGNa = RPARAM(47) 
      PNa = RPARAM(48)
      Istim = RPARAM(49)
      Farad = 96.5d3 !C/mol
c      RTONF = (Temp+273.d0)*86.16d-3 !mV
      
      RETURN
      END 


      SUBROUTINE DEFINE_NOBLE98(PARAM)

C#### Subroutine: DEFINE_NOBLE98
C###  Description:
C###    Initialise Noble '98 variables/parameters

      IMPLICIT NONE
      !passed variables
      REAL*8 PARAM(*)
      !common blocks
      REAL*8    OH5_kcachoff,OH5_kmk1,OH5_kmK,
     '          OH5_kmNa,OH5_kNaCa,OH5_kmCa,
     '          OH5_k1,OH5_k2,OH5_k3,OH5_k4,
     '          OH5_kATP,OH5_KD,OH5_kkNa,
     '          OH5_kdsoff,OH5_kmCads,OH5_kdecay,
     '          OH5_Rdecay,OH5_INaKmax
      COMMON /OH5_PARAM1/
     '  OH5_kcachoff,OH5_kmk1,OH5_kmK,
     '  OH5_kmNa,OH5_kNaCa,OH5_kmCa,
     '  OH5_k1,OH5_k2,OH5_k3,OH5_k4,
     '  OH5_kATP,OH5_KD,OH5_kkNa,
     '  OH5_kdsoff,OH5_kmCads,OH5_kdecay,
     '  OH5_Rdecay,OH5_INaKmax
      REAL*8    OH5_GNa,OH5_GbK,OH5_GK1,OH5_GbNa,
     '          OH5_GpNa,OH5_GbCa,
     '          OH5_GKr1,OH5_GKr2,OH5_GKs,OH5_GKATP,OH5_GKACh,
     '          OH5_PCa,OH5_PCaK,OH5_PCaNa,OH5_PKNa
      COMMON /OH5_PARAM2/
     '  OH5_GNa,OH5_GbK,OH5_GK1,OH5_GbNa,
     '  OH5_GpNa,OH5_GbCa,
     '  OH5_GKr1,OH5_GKr2,OH5_GKs,OH5_GKATP,OH5_GKACh,
     '  OH5_PCa,OH5_PCaK,OH5_PCaNa,OH5_PKNa
      REAL*8    OH5_Cab,OH5_Ko,OH5_Nao,
     '          OH5_FractICaL,OH5_FractINaCa,
     '          OH5_DIFFCa,OH5_dNaCa,OH5_gama,OH5_nNaCa,
     '          OH5_kcyCa,OH5_kxcs,OH5_ksrCa,OH5_Cm,
     '          OH5_ALPHA12,OH5_BETA12
      COMMON /OH5_PARAM3/
     '  OH5_Cab,OH5_Ko,OH5_Nao,
     '  OH5_FractICaL,OH5_FractINaCa,
     '  OH5_DIFFCa,OH5_dNaCa,OH5_gama,OH5_nNaCa,
     '  OH5_kcyCa,OH5_kxcs,OH5_ksrCa,OH5_Cm,
     '  OH5_ALPHA12,OH5_BETA12
      REAL*8    OH5_F,OH5_R,OH5_T,
     '          OH5_Vecs,OH5_radius,OH5_length,
     '          OH5_Vup,OH5_Vrel,OH5_kmCa2,
     '          OH5_Mtrop,OH5_Ctrop,OH5_alfatrop,
     '          OH5_betatrop,OH5_gamatropSL,
     '          OH5_gamaSRSL,OH5_sacSL
      COMMON /OH5_PARAM4/
     '  OH5_F,OH5_R,OH5_T,
     '  OH5_Vecs,OH5_radius,OH5_length,
     '  OH5_Vup,OH5_Vrel,OH5_kmCa2,
     '  OH5_Mtrop,OH5_Ctrop,OH5_alfatrop,
     '  OH5_betatrop,OH5_gamatropSL,
     '  OH5_gamaSRSL,OH5_sacSL
      REAL*8    OH5_G_Ca_stretch,OH5_G_Na_stretch,OH5_G_K_stretch,
     '          OH5_G_Ns_stretch,OH5_G_An_stretch,
     '          OH5_E_Ns_stretch,OH5_E_An_stretch
      COMMON /OH5_PARAM5/
     '  OH5_G_Ca_stretch,OH5_G_Na_stretch,OH5_G_K_stretch,
     '  OH5_G_Ns_stretch,OH5_G_An_stretch,
     '  OH5_E_Ns_stretch,OH5_E_An_stretch
      REAL*8    OH5_alfaS,OH5_alfaV,
     '          OH5_alfaACh,OH5_betaACh,
     '          OH5_SLhst,OH5_IThst
      COMMON /OH5_PARAM6/
     '  OH5_alfaS,OH5_alfaV,
     '  OH5_alfaACh,OH5_betaACh,
     '  OH5_SLhst,OH5_IThst
      REAL*8 OH5_kdsdecay,OH5_Jsrleak
      COMMON /OH5_PARAM7/
     '  OH5_kdsdecay,OH5_Jsrleak
      REAL*8 OH5_Vcell,OH5_Vi,OH5_Vds,OH5_VSRup
      COMMON /OH5_VOLUMES/
     '  OH5_Vcell,OH5_Vi,OH5_Vds,OH5_VSRup
      REAL*8 OH5_STIMSIZE,OH5_FREQ
      COMMON /OH5_STIM/ 
     '  OH5_STIMSIZE,OH5_FREQ
      !local variables
      REAL*8 PI

      OH5_STIMSIZE   = PARAM( 1)
      OH5_Cm         = PARAM( 2)
      OH5_F          = PARAM( 3)
      OH5_R          = PARAM( 4)
      OH5_T          = PARAM( 5)
      OH5_Rdecay     = PARAM( 6)
      OH5_kdsoff     = PARAM( 7)
      OH5_Nao        = PARAM( 8)
      OH5_Ko         = PARAM( 9)
      OH5_Vecs       = PARAM(10)
      OH5_Vup        = PARAM(11)
      OH5_Vrel       = PARAM(12)
      OH5_kdsdecay   = PARAM(13)
      OH5_Mtrop      = PARAM(14)
      OH5_alfatrop   = PARAM(15)
      OH5_betatrop   = PARAM(16)
      OH5_Ctrop      = PARAM(17)
      OH5_gK1        = PARAM(18)
      OH5_kmK1       = PARAM(19)
      OH5_gKr1       = PARAM(20)
      OH5_gKr2       = PARAM(21)
      OH5_gKs        = PARAM(22)
      OH5_gNa        = PARAM(23)
      OH5_gpNa       = PARAM(24)
      OH5_gbNa       = PARAM(25)
      OH5_PCa        = PARAM(26)
      OH5_PCaK       = PARAM(27)
      OH5_PCaNa      = PARAM(28)
      OH5_gbCa       = PARAM(29)
      OH5_INaKmax    = PARAM(30)
      OH5_kmK        = PARAM(31)
      OH5_kmNa       = PARAM(32)
      OH5_FractINaCa = PARAM(33)
      OH5_kNaCa      = PARAM(34)
      OH5_gama       = PARAM(35)
      OH5_dNaCa      = PARAM(36)
      OH5_kcyCa      = PARAM(37)
      OH5_kxcs       = PARAM(38)
      OH5_ksrCa      = PARAM(39)
      OH5_kmCa2      = PARAM(40)
      OH5_Jsrleak    = PARAM(41)
      OH5_kmCa       = PARAM(42)
      OH5_kmCads     = PARAM(43)
      OH5_PKNa       = PARAM(44)
      OH5_FREQ       = PARAM(45)
      OH5_radius     = PARAM(46)
      OH5_length     = PARAM(47)
      OH5_ALPHA12    = PARAM(48)
      OH5_BETA12     = PARAM(49)

      OH5_FractICaL  = 1.d0   ! Fraction of ICaL -> sub-space

! Cell space volumes
      PI = 4.0d0*ATAN(1.0d0)
      OH5_Vcell = 1.d-9*PI*OH5_radius**2*OH5_length          !uL
      OH5_Vi    = (1.d0-OH5_Vecs-OH5_Vup-OH5_Vrel)*OH5_Vcell !uL
      OH5_Vds   = 0.1d0*OH5_Vi                               !uL
      OH5_VSRup = OH5_Vcell*OH5_Vup                          !uL

C      IF (DABS(PARAM(62)).LT.1.0d-3) THEN
C        OH5_STIMSIZE = -3.0d0
C      ELSE
C        OH5_STIMSIZE = PARAM(62)
C      ENDIF
C      OH5_FREQ = PARAM(63)
C
C      OH5_Cm       = 95.0d-6 !uF
C
C      OH5_kcachoff = 1.d-3   !mM
C      OH5_kmk1     = 10.d0   !mM
C      OH5_kmK      = 1.d0    !mM
C      OH5_kmNa     = 40.d0   !mM
C      OH5_kNaCa    = 5.0d-4 !1.d-4   !nA
C      OH5_kmCa     = 5.0d-4 !1.d-3   !mM
C      OH5_k1       = 1.2d4   !mM
C      OH5_k2       = 100.d0  !mM
C      OH5_k3       = 60.d0   !mM
C      OH5_k4       = 25.d0   !mM
C      OH5_kATP     = 0.1d0   !mM
C      OH5_KD       = 0.13d-3 !mM ACh !uM ACh
C      OH5_kkNa     = 20.d0   !mM
C      OH5_kdsoff   = 1.d-3   !mM
C      OH5_kmCads   = 1.d-2   !mM
C      OH5_kdecay   = 100.d-3 ! s ???? !ms
c      OH5_Rdecay   = 20.d0   !mM/s
C      OH5_INaKmax  = 0.7d0 !0.14d0  !nA
C      OH5_GNa      = 2.5d0  !0.5d0   !uS
C      OH5_GbK      = 0.0d0  !1.7d-3  !uS
C      OH5_GK1      = 0.5d0  !1.7d-2  !uS
C      OH5_GbNa     = 6.0d-4 !1.2d-4  !uS
C      OH5_GpNa     = 4.d-3   !uS
C      OH5_GbCa     = 2.5d-4 !5.d-5   !uS
C      OH5_GKr1     = 2.8d-3  !uS
C      OH5_GKr2     = 1.7d-3  !uS
C      OH5_GKs      = 3.2d-3  !uS
C      OH5_GKATP    = 0.0d0   !uS - value not given
C      OH5_GKACh    = 5.d-5   !uS
C      OH5_PCa      = 0.1 !5.d-2   !nA/mM
C      OH5_PCaK     = 2.d-3   !nA/mM
C      OH5_PCaNa    = 2.d-3   !nA/mM
C      OH5_PKNa     = 3.d-2   !nA/mM
C      OH5_Cab        = 2.d0  !mM
c      OH5_Ko         = 4.d0    !mM
c      OH5_Nao        = 140.d0  !mM
c      OH5_FractICaL  = 1.d0   ! Fraction of ICaL -> sub-space
c      OH5_FractINaCa = 1.d-3  !  "   "    " INaCa -> "  "   "
c      OH5_DIFFCa     = 5.d-4   !NA ?
c      OH5_dNaCa      = 0.0d0 !1.d-4   !
C      OH5_gama       = 0.5d0   !
C      OH5_nNaCa      = 3.d0    !
C      OH5_kcyCa      = 3.d-4   !mM
C      OH5_kxcs       = 0.4d0   !mM
C      OH5_ksrCa      = 0.5d0   !mM
c      OH5_F         = 96485.0d0  !Coulombs/mole
c      OH5_R         = 8314.41d0  !mJoules/mole/degK
c      OH5_T            = 310.d0  !degK
c      OH5_Vecs         = 0.4d0   !
c      OH5_radius       = 10.d0 !um
c      OH5_length       = 80.d0 !um
c      OH5_Vup          = 1.d-2   !
c      OH5_Vrel         = 1.d-1   !
c      OH5_kmCa2        = 250.d0  !nA/mM
c      OH5_Mtrop        = 0.02d0  !mM
C      OH5_Ctrop        = 0.15d0  !mM
C      OH5_alfatrop     = 1.0d5 ! mM/s   !5.d3    !/s
C      OH5_betatrop     = 2.d2    !/s
C      OH5_gamatropSL   = 2.5d0   !
C      OH5_gamaSRSL         = 1.5d0   !
C      OH5_sacSL        = 2.5d0   !
C      !OH5_GCa_stretch = 0.d0  !uS
C      !OH5_GNa_stretch = 0.d0   !uS
C      !OH5_GK_stretch  = 0.d0   !uS
C      !OH5_GNs_stretch = 0.d0   !uS
C      !OH5_GAn_stretch = 0.d0   !uS
C      !OH5_ENs_stretch = 0.d0   !mV
C      !OH5_EAn_stretch = 0.d0   !mV
C      OH5_alfaS     = 2.d0  !
C      OH5_alfaV     = 2.d0    !
C      OH5_alfaACh   = 0.5d0   !/s
C      OH5_betaACh   = 0.5d0   !/s
C      OH5_SLhst     = 2.d0    !um
C      OH5_IThst     = 1.d0    !au ?

      RETURN
      END


      SUBROUTINE DEOXS1(IPARAM_1)

C#### Subroutine: DEOXS1
C###  Description:
C###    Defines Oxsoft Heart control modes:
C###    ( 1) Operating_mode
C###    ( 2) Current/voltage_mode
C###    ( 3) Preparation_type
C###    ( 4) Diffusion_space_type
C###    ( 5) Output_files
C###    ( 6) Current_display
C###    ( 7) Terminal_output
C###    ( 8) Interrupt_facility
C###    ( 9) Switch_facility

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      INTEGER IPARAM_1(9)

      IF(DEBUG) write(*,'('' >>>call deoxs1'')')

      MODE    = IPARAM_1(1) !Operating_mode
      PMODE   = IPARAM_1(2) !Current/voltage_mode
      PREP    = IPARAM_1(3) !Preparation_type
      SPACE   = IPARAM_1(4) !Diffusion_space_type
      OUT     = IPARAM_1(5)
      IDISP   = IPARAM_1(6)
      DISP    = IPARAM_1(7)
      CONTIN  = IPARAM_1(8)
      SWITCH  = IPARAM_1(9)

      RETURN
      END


      SUBROUTINE DEOXS2(IPARAM_2,LPARAM_2,RPARAM_2)

C#### Subroutine: DEOXS2
C###  Description:
C###    Defines Oxsoft Heart model modes:
C###    ( 1) Choice of IK equations
C###    ( 2) Choice of ICa equations
C###    ( 3) Selectivity for fast Ca channel
C###    ( 4) Choice of function for INaCa
C###    ( 5) Choice of Ito equations
C###    ( 6) [Ca]i activation (if >0 another 9 real parameters are reqd)
C###    ( 7) Voltage dependence of repriming
C###    ( 8) Choice of INa equations
C###    ( 9) Ca buffering
C###    (10) SR Ca-sequestration
C###    (11) Computation of [Ca]o
C###    (12) Mitochondrial Ca
C###    (13) Kinetics of If

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      INTEGER IPARAM_2(13)
      LOGICAL LPARAM_2(5)
      REAL*8 RPARAM_2(9)

      IF(DEBUG) write(*,'('' >>>call deoxs2'')')

      KMODE   = IPARAM_2( 1)
      CAMODE  = IPARAM_2( 2)
      CANMODE = IPARAM_2( 3)
      NMODE   = IPARAM_2( 4)
      TOMODE  = IPARAM_2( 5)
      AMODE   = IPARAM_2( 6)
      RMODE   = IPARAM_2( 7)
      NAMODE  = IPARAM_2( 8)
      BMODE   = IPARAM_2( 9)
      SRMODE  = IPARAM_2(10)
      CAOMODE = IPARAM_2(11)
      MMODE   = IPARAM_2(12)
      YMODE   = IPARAM_2(13)

      CASS        = LPARAM_2(1)
      GNASS       = LPARAM_2(2)
      BUFFAST     = LPARAM_2(3)
      SLPUMPMODE  = LPARAM_2(4)
      CONTRACTMODE= LPARAM_2(5)

      CACT(1)   = RPARAM_2( 1)
      MIK1      = RPARAM_2( 2)
      CACT(2)   = RPARAM_2( 3)
      MIK       = RPARAM_2( 4)
      CACT(3)   = RPARAM_2( 5)
      MIB       = RPARAM_2( 6)
      CACT(4)   = RPARAM_2( 7)
      MITO      = RPARAM_2( 8)
      CACT(5)   = RPARAM_2( 9)

      RETURN
      END


      SUBROUTINE DEOXS3(TYPE,RPARAM)

C#### Subroutine: DEOXS3
C###  Description:
C###    Defines Oxsoft Heart parameters. ID_PARAM is:
C###    (1) Ca_buffering parameters
C###    (2) Ca_sequestration parameters
C###    (3) Concentrations
C###    (4) Diffusion_spaces
C###    (5) Ion_channels
C###    (6) Other_parameters
C###    (7) Pump_exchange
C###    (8) Single_cell
C###    (9) Time_steps

      IMPLICIT NONE
      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      INTEGER TYPE
      REAL*8 RPARAM(21,*)

      IF(DEBUG) write(*,'('' >>>call deoxs3'')')

      IF(TYPE.EQ.1) THEN !Ca_buffering parameters

        CTROP     = RPARAM( 1,1) !Ca bound to troponin
        MTROP     = RPARAM( 2,1)
        CALMOD    = RPARAM( 3,1) ![calmodulin]
        CAMGTROP  = RPARAM( 4,1)
        KCALON    = RPARAM( 5,1)
        KCALOFF   = RPARAM( 6,1)
        KTROCON   = RPARAM( 7,1)
        KTROCOFF  = RPARAM( 8,1)
        KTROMON   = RPARAM( 9,1)
        KTROMOFF  = RPARAM(10,1)
        KTROCMON  = RPARAM(11,1)
        KTROCMOFF = RPARAM(12,1)
        KCACHOFF  = RPARAM(13,1)

      ELSE IF(TYPE.EQ.2) THEN !Ca_sequestration parameters

        V12       = RPARAM( 1,2)
        V13       = RPARAM( 2,2)
        CA12M     = RPARAM( 3,2)
        VSURFCA   = RPARAM( 4,2)
        SRLEAK    = RPARAM( 5,2) !Ca release from release store
        KSLPUMP   = RPARAM( 6,2) !scaling constant for sarcolemma Ca pump
        KMSLPUMP  = RPARAM( 7,2)
        KCYCA     = RPARAM( 8,2)
        KSRCA     = RPARAM( 9,2)
        KXCS      = RPARAM(10,2)
        TAUREL    = RPARAM(11,2) !release time constant
        NREL      = RPARAM(12,2)
        CA12      = RPARAM(13,2) ![Ca] in uptake store
        CA13      = RPARAM(14,2) ![Ca] in release store
        REPRIM    = RPARAM(15,2)

      ELSE IF(TYPE.EQ.3) THEN !Concentrations

        KB        = RPARAM( 1,3) !bulk external [K]
        KI        = RPARAM( 2,3) ![K]i
        NAI       = RPARAM( 3,3) ![Na]i
        CAI       = RPARAM( 4,3) ![Ca]i
        CAO       = RPARAM( 5,3) ![Ca]o
        CALIM     = RPARAM( 6,3) ![Ca]i-minimum
        NAO       = RPARAM( 7,3) ![Na]o
        KNACA     = RPARAM( 8,3) !scaling factor for Na-Ca exchange
        KM        = RPARAM( 9,3) !constant for K activation of Na-K pump
        KMNA      = RPARAM(10,3) !constant for K activation of Na-K pump
        KMK1      = RPARAM(11,3)
        KMTO      = RPARAM(12,3)
        KMF       = RPARAM(13,3)
        KMCA      = RPARAM(14,3)
        MG        = RPARAM(15,3)

      ELSE IF(TYPE.EQ.4) THEN !Diffusion_spaces

        PF        = RPARAM( 1,4)
        TORT      = RPARAM( 2,4)

      ELSE IF(TYPE.EQ.5) THEN !Ion_channels

        GNA       = RPARAM( 1,5) !maximum fast Na conductance
        PCA       = RPARAM( 2,5) !Ca permeability of fast Ca channel
        TAUF2     = RPARAM( 3,5) 
        PCA3      = RPARAM( 4,5)
        IKM       = RPARAM( 5,5)
        GFNA      = RPARAM( 6,5)
        GFK       = RPARAM( 7,5)
        GTO       = RPARAM( 8,5)
        SHIFTTO   = RPARAM( 9,5)
        GB        = RPARAM(10,5) !background conductance (used when gbNa=gbK)
        GBNA      = RPARAM(11,5)
        GBK       = RPARAM(12,5)
        GBCA      = RPARAM(13,5) 
        GK1       = RPARAM(14,5) !maximum conductance for IK1
        SHIFTK1   = RPARAM(15,5)
        GNAK      = RPARAM(16,5)
        PCA2      = RPARAM(17,5) 
        PCAK      = RPARAM(18,5) !K permeability of ICa channels
        IACHM     = RPARAM(19,5)
        ISCAL     = RPARAM(20,5)
        PNAK      = RPARAM(21,5)

      ELSE IF(TYPE.EQ.6) THEN !Other_parameters

        C         = RPARAM( 1,6)
        EC        = RPARAM( 2,6)
        PULSESIZE = RPARAM( 3,6)
        IPULSESIZE= RPARAM( 4,6)
        ON        = RPARAM( 5,6)*1.0d-3 !DPN 24/02/98 - convert from ms
        OFF       = RPARAM( 6,6)*1.0d-3
        REP       = RPARAM( 7,6)*1.0d-3
        STIMSIZE  = RPARAM( 8,6)
        VOL       = RPARAM( 9,6)
        RS        = RPARAM(10,6)
        RE        = RPARAM(11,6)
        AMP       = RPARAM(12,6)
        DX        = RPARAM(13,6)
        NINTERVAL = RPARAM(14,6)
        PREPLENGTH= RPARAM(15,6)

      ELSE IF(TYPE.EQ.7) THEN !Pump_exchange

        PUMP      = RPARAM( 1,7)
        NNAK      = RPARAM( 2,7) !Na-K pump ratio
        INACAM    = RPARAM( 3,7)
        NNACA     = RPARAM( 4,7) !Na-Ca exchange ratio
        SNACA     = RPARAM( 5,7) !scaling factor for INaCa
        DNACA     = RPARAM( 6,7)
        YNACA     = RPARAM( 7,7)
        SCALE     = RPARAM( 8,7)
    
      ELSE IF(TYPE.EQ.8) THEN !Single_cell

        NAPE      = RPARAM( 1,8)
        CAPE      = RPARAM( 2,8)
        KPE       = RPARAM( 3,8)
        TNAP      = RPARAM( 4,8)
        TCAP      = RPARAM( 5,8)
        TKP       = RPARAM( 6,8)

      ELSE IF(TYPE.EQ.9) THEN !Time_steps

        TIMESCALE = RPARAM( 1,9) !scale factor for time (eg 1000 -> o/p in ms)
C ??? DPN 19/11/97 - convert from ms to seconds ???
        TSTART    = RPARAM( 2,9)*1.0d-3 !initial value of t
        TEND      = RPARAM( 3,9)*1.0d-3 !final value of t
        DT        = RPARAM( 4,9)*1.0d-3 !time increment
        TABT      = RPARAM( 5,9)*1.0d-3 !tabulation interval
        TPS       = RPARAM( 6,9)*1.0d-3 !value of t when stimulus starts
        TP        = RPARAM( 7,9)*1.0d-3 !duration of stimulus
        
      ENDIF

      RETURN
      END


      SUBROUTINE DESOL(SUB_NAME,NT_EQN,TBEG,TEND,Y,IFLAG)

C#### Subroutine: DESOL
C###  Description:
C###    <html>
C###    This is the driver procedure of a set of procedures to 
C###    integrate a set of differential equations of the form 
C###    dy/dt = f(y,t) from t = TBEG to t = TEND. The actual integration
C###    proceeds to the first value of t exceeding tout and the values 
C###    at TEND are obtained by interpolation.
C###    The variables are:
C###    <ul>
C###    <li>NT_EQN  -  the number of equations to be integrated
C###    <li>IFLAG -  indicates state of program and is 0 on first call
C###      <ul>
C###      <li>1 on all subsequent normal returns from desol
C###      <li>2 -required accuracy impossible with minimum step length
C###      <li>3 -invalid input
C###      <li>4 -solution required more than 2000 function evaluations
C###      <li>5 -voltage exceeds +- 200 mv
C###      </ul>
C###    <li>TBEG  -  value of t when desol is called
C###    <li>TEND  -  value of t when desol returns to main program
C###    <li>Y     -  array of values of dependent variables
C###    <li>SAVE  -  a stored array of variables to be saved between
C###    calls to desol, including:
C###      <ul>
C###      <li>SAVE7 = step size on next step
C###      <li>SAVE8 = step size used on last step
C###      <li>ISAVE -  array of stored integers:
C###      <li>ISAVE2 = number of function evaluations during last call
C###      <li>ISAVE3 = number of steps so far
C###      </ul>
C###    </ul></html>

      IMPLICIT NONE
c      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      INTEGER IFLAG,NT_EQN
      REAL*8 TBEG,TEND,Y(*)
      CHARACTER SUB_NAME*(*)
!     Local Variables
      INTEGER ISAVE1,ISAVE2,ISAVE3,ISAVE4,KFLAG,NO_EQN !,I,LD1
      REAL*8 F_SAVE(99),SAVE10,SAVE11,SAVE12,SAVE5,SAVE6,SAVE7,
     '  SAVE8,SAVEN2(99),SAVEN3(99),SAVY(99),TEND_SAVE,Y_SAVE(99)!,F(99)

C      write(*,'('' >>>call desol - i.e. Membrane solving at '',D11.5)')
C     '  TBEG

      ISAVE3 = 0 !DPN 17/12/97 - FTNCHK error ???
      ISAVE2=0
      KFLAG=1

C **  Set maximum allowed step length.
      SAVE5=0.5d0*(TEND-TBEG)

      IF(TEND.LT.TBEG) THEN
        KFLAG=2
        SAVE5=1.0d6
      ENDIF

C **  In first call, initialize values of y array.
      IF(IFLAG.EQ.0) THEN
        DO NO_EQN=1,NT_EQN
          Y_SAVE(NO_EQN)=Y(NO_EQN)
        ENDDO
      ENDIF

      IF(SUB_NAME(1:2).EQ.'DN') THEN
!       IF(DOP) THEN
!          WRITE(*,'('' Y( 1..10):'',10E11.3)') (Y(I),I= 1,10)
!          WRITE(*,'('' Y(11..20):'',10E11.3)') (Y(I),I=11,20)
!          WRITE(*,'('' Y(21..30):'',10E11.3)') (Y(I),I=21,30)
!       ENDIF
      ENDIF

C **  Call Adams to advance one step unless y(tout) interpolatable.
      TEND_SAVE=TBEG
      DO WHILE(((TEND.GT.TEND_SAVE).OR.(IFLAG.EQ.0)).AND.(IFLAG.LT.2))
        IF(DABS(Y_SAVE(1)).GT.200.0d0) IFLAG=5 !if voltage outside +-200mV
        IF(IFLAG.LT.2) THEN
          CALL ADAMS(SUB_NAME,NT_EQN,TBEG,SAVE5,SAVE6,SAVE7,SAVE8,
     '      TEND_SAVE,SAVE10,SAVE11,SAVE12,Y_SAVE,F_SAVE,SAVEN2,SAVEN3,
     '      ISAVE1,ISAVE2,ISAVE3,ISAVE4,IFLAG)
        ENDIF

C **    Set y to current value of y(t).
        DO NO_EQN=1,NT_EQN
          Y(NO_EQN)=Y_SAVE(NO_EQN)
C dpn 17/02/98          F(NO_EQN)=F_SAVE(NO_EQN)
        ENDDO

C **    Check for abnormal return from Adams.
C ***   ISAVE2 = no. function evaluations
c        IF((IFLAG.EQ.2).OR.(ISAVE2.GT.2000).OR.(KFLAG.EQ.2)) THEN
        IF((IFLAG.EQ.2).OR.(ISAVE2.GT.30000).OR.(KFLAG.EQ.2)) THEN
          WRITE(*,'('' ***Abnormal return from ADAMS***'')')
          WRITE(*,'(''    TBEG = '',10D11.3)') TBEG
          WRITE(*,*) IFLAG,ISAVE2,KFLAG
          TEND=TEND_SAVE
          IF(ISAVE2.GT.30000) IFLAG=4
        ENDIF
      ENDDO

C **  Interpolate value for tend.
      IF((IFLAG.EQ.1).AND.(KFLAG.EQ.1)) THEN
        CALL INTERP(NT_EQN,TEND,SAVE7,TEND_SAVE,SAVE10,SAVY,Y_SAVE,
     '    F_SAVE,SAVEN2,SAVEN3)
        DO NO_EQN=1,NT_EQN
          Y(NO_EQN)=SAVY(NO_EQN)
C dpn 17/02/98          F(NO_EQN)=F_SAVE(NO_EQN)
        ENDDO
      ENDIF

      RETURN
      END


      SUBROUTINE DFN_CHANGE(T01,Y,IFLAG)

C#### Subroutine: DFN_CHANGE
C###  Description:
C###    Checks time to change parameter values.

      IMPLICIT NONE
      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs002.cmn'
      INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      INTEGER IFLAG
      REAL*8 T01,Y(*)
!     Local Variables
      INTEGER NEQND
      REAL*8 PULSELENGTH,TAB,TSWITCH,TPOFF

!MLB initialising variables used before set
      NEQND=0
      TAB=0.0d0
      TSWITCH=0.0d0

!      write(*,'('' >>>call dfn_change'')')

C *** DPN 20/01/98 - FTNCHEK error (used before set)
c      IF (CHANGE_FIX.EQ.0) THEN
c        NEQND = 0
c        TAB = 0.0d0
c        TSWITCH = 0.0d0
c        CHANGE_FIX = 1
c      ENDIF

C?????DB.  Is this correct ?
C      IF(SWITCH) THEN
      IF(0.LT.SWITCH) THEN
        IF(T01.GE.TSWITCH) THEN
          IF(MODE.EQ.1) THEN
            MODE=2
            CLAMP=.TRUE.
          ELSE
            MODE=1
            CLAMP=.FALSE.
          ENDIF
          TSWITCH=TEND+1.0d0
          IFLAG=0
          TAB=TAB-TABT*TABT/DT
          IF(DEBUG) WRITE(*,*) ' Switch operated'
        ENDIF
      ENDIF

      IF(MODE.EQ.1) THEN
        IF(.NOT.GNASS) THEN
          IF(DABS(F1).GT.DVTEST.AND.
     '      .NOT.(F1.LT.0.0d0.AND.(Y(1).LT.0.0d0.OR.PREP.NE.1))) THEN
            IF(.NOT.FAST) THEN
              IF(DEBUG) 
     '          WRITE(*,*) ' >>Sodium activation is now time-dependent'
            ENDIF
            FAST=.TRUE.
            DT=DTS
            IF(F1.GT.0.0d0) THEN
              DT=DTSS
              NEQND=NEQND+1
            ENDIF
          ENDIF
        ELSE IF(GNASS) THEN
          IF(Y(1).LT.0.0d0.OR.PREP.NE.1.OR.DABS(F1).LT.DVTEST) THEN
            IF(FAST) THEN
              IF(DEBUG) 
     '          WRITE(*,*) ' >>Sodium activation now instantaneous'
            ENDIF
            FAST=.FALSE.
            DT=DTL
            NEQND=0
          ENDIF
        ENDIF

        IF(T01.GE.TPS) THEN
          IF(DEBUG) WRITE(*,*) ' >>Stimulus on'
c          write(*,*) 'Stimulus on'
c          write(*,'('' T0='',D11.3,'' TPS='',D11.3,'' TP='',d11.3)') 
c     '      t0,TPS,TP
c          write(*,'('' STIMSIZE = '',D11.3)') STIMSIZE
          IFLAG=0
          STIM=STIMSIZE
          TPOFF=T01+TP
          TPS=TEND+1.0d0
          DT=DTSS
        ENDIF
        IF(T01.GT.TPOFF) THEN
          IF(DEBUG) WRITE(*,*) ' >>Stimulus off'
c          write(*,*) 'Stimulus off'
c          write(*,'('' T0='',D11.3,'' TPS='',D11.3,'' TP='',d11.3)') 
c     '      t0,TPS,TP
          STIM=0.0d0
          TPOFF=TEND+1.0d0
        ENDIF

        IF(T01.GT.ON) THEN
          IF(DEBUG) WRITE(*,*) ' >>Current pulse on'
c          write(*,*) 'Current pulse on'
c          write(*,'('' T0='',D11.3,'' IPULSESIZE='',D11.3,)') t0,
c     '      ipulsesize
          IPULSE=IPULSESIZE
          ON =T01+REP
          PULSELENGTH=TP !?? check PJH 15Jan91
          OFF=T01+PULSELENGTH
c          write(*,'('' ON='',D11.3,'' OFF='',D11.3,)') on,off
          IF(PULSELENGTH.LT.5.0d0*DT) DT=DTSS
          IFLAG=0
        ENDIF
        IF(T01.GT.OFF) THEN
          IF(DEBUG) WRITE(*,*) ' >>Current pulse off'
c          write(*,*) 'Current pulse off'
c          write(*,'('' T0='',D11.3,'' IPULSESIZE='',D11.3,)') t0,
c     '      ipulsesize
          IPULSE=0.0d0
          OFF=OFF+REP
c          write(*,'('' ON='',D11.3,'' OFF='',D11.3,)') on,off
        ENDIF

        IF(DABS(F1).GT.20.0d0*DVTEST) DT=0.2d0*DTSS
        IF(NEQND.EQ.1) IFLAG=0
        IF(STIM.EQ.0.0) THEN
          IF(IPULSE.EQ.0.0d0) THEN
            IF(Y(1).LT.-85.0d0) THEN
              FAST=.FALSE.
              DT=DTL
              NEQND=0
            ENDIF
          ENDIF
        ENDIF

      ELSE IF(MODE.EQ.2) THEN

      ELSE IF(MODE.EQ.3) THEN

      ELSE IF(MODE.GE.4) THEN

      ENDIF

      RETURN
      END


      SUBROUTINE DFN_CONCENTRATIONS(Y,F)

C#### Subroutine: DFN_CONCENTRATIONS
C###  Description:
C###    Computes ion concentrations for HEART

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs001.cmn'
      INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs004.cmn'
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      REAL*8 F(*),Y(*)!,T
!     Local Variables
      INTEGER L,NNREL
      REAL*8 CAFMIT,ENA,
     '  FCTROP,FCYT,FMTROP,FSR,
     '  IMCA,
     '  IMNA,IREL,ITRAN,IUP,
     '  Z2,Z3,Z4,Z5!,DCNP,DCP,FCYTFREE,FSRFREE
C CACH(10),ECH(10),EKC(0:11),IACHC(0:11),IBKC(0:11),ICAKC(0:11),IFKC(0:11),IFNAC(0:11),IK1C(0:11),IKC(0:11),IMKC(0:11),IMKSAV(0:11),IPC(0:11),ISI,ITOC(0:11),ITOT,KCH(10),NACH(10),PUMPCH(10),TCH(10),YSAV(0:11),
      LOGICAL CONTINUE

!      write(*,'('' >>>call dfn_concentrations'')')

C *** d(Na)i/dt = net sodium current/(F x internal volume)
C *** assume fraction of ibna carried by Na prop. to (Na)o

      IMNA = INA+(IBNA*NAO/140.0d0)+IFNA+ICANA+(IP*NNAK/(NNAK-1.0d0))
      IF(NNACA.GT.2.0d0) IMNA = IMNA+INACA*NNACA/(NNACA-2.0d0)
      F(2) = -IMNA/VIF

C *** Add Na change due to perfusion

      IF(SPACE.EQ.5) F(2) = F(2)+(NAPE-Y(2))/TNAP

C *** For 3 compartment case set d(K)c/dt = iK/VF - K diffusion

      IF(SPACE.EQ.3) THEN
        F(3) = IMK/VF-PF*(Y(3)-KB)
      ELSE
        F(3) = 0.0d0
      ENDIF

C *** When ecs is not restricted, set [K]c = K[b]

      IF((SPACE.EQ.4).OR.(SPACE.EQ.5)) Y(3) = KB

C *** Compute mitochondrial calcium

      IF(MMODE.EQ.1) THEN
        Z1 = (KCMUP/Y(4))**NCMUP
        Z1 = ALPHA(25)/(1.0d0+Z1)
        Z2 = (KNMREL/Y(2))**NNMREL
        Z3 = 1.0d0/(1.0d0+KCMREL/Y(25))
        Z2 = BETA(25)*Z3/(1.0d0+Z2)
        Z4 = 0.0639d0*Z3*BETA(25)
        Z1 = Z1-Z2-Z4
        F(25) = Z1/VMIT
        CAFMIT = Z1/VI
      ELSE
        F(25) = 0.0d0
      ENDIF

C *** Compute calcium stores

      IF(.NOT.CASS) THEN

        IF(BMODE.EQ.0) THEN           ! D-N formulation
          DO L = 20,23
            F(L) = 0.0d0
          ENDDO
        ELSE IF(BMODE.EQ.1) THEN      ! Calgary formulation
          F(20) = ALPHA(20)*Y(4)*(1.0d0-Y(20))-BETA(20)*Y(20)
          F(21) = ALPHA(21)*Y(4)*(1.0d0-Y(21))-BETA(21)*Y(21)
          F(22) = ALPHA(22)*MG*(1.0d0-Y(22)-Y(23))-BETA(22)*Y(22)
          F(23) = ALPHA(23)*Y(4)*(1.0d0-Y(22)-Y(23))-BETA(23)*Y(23)
        ELSE IF(BMODE.EQ.2) THEN      ! Hilgemann formulation
          IF(BUFFAST) THEN
            F(31)= 0.0d0
            Y(31)= Y(4) + Y(20) + Y(21)
            F(21)=(CTROP-Y(21))*ALPHA(21)*Y(4)-Y(21)*BETA(21)
            F(20)=(MTROP-Y(20))*ALPHA(20)*Y(4)-Y(20)*BETA(20)
          ELSE

C ***       Calculate steady state buffers using the Hilgemann formulation.
            CONTINUE=.TRUE.
            DO WHILE(CONTINUE)
              Z2 = Y(21)
              Z3 = Y(20)
              Y(21) = (CTROP*(Y(20)-Y(31)))/
     '          (Y(21)-Y(31)-CTROP+Y(20)-BETA(21)/ALPHA(21))
              Y(20) = (MTROP*(Y(21)-Y(31)))/
     '          (Y(20)-Y(31)-MTROP+Y(21)-BETA(20)/ALPHA(20))
              IF((DABS((Z2-Y(21))/Y(21))+DABS((Z3-Y(20))
     '          /Y(20))).LT.0.001d0) CONTINUE=.FALSE.
            ENDDO
            Y(4)  = Y(31)-Y(20)-Y(21)
            F(4)  = 0.0d0
            F(21) = 0.0d0
            F(20) = 0.0d0
          ENDIF
          CACHOFF = Y(4)/(KCACHOFF+Y(4))
          FCTROP  = Y(21)/CTROP
          FMTROP  = Y(20)/MTROP
          F(22)   = 0.0d0
          F(23)   = 0.0d0
        ENDIF

        IF(SRMODE.EQ.0) THEN          ! SR calcium release and transfer zero
          DO L = 12,14
            F(L) = 0.0d0
            Y(L) = 0.0d0
          ENDDO
          DO L = 26,28
            F(L) = 0.0d0
            Y(L) = 0.0d0
          ENDDO
          IUP   = 0.0d0
          IREL  = 0.0d0
          ITRAN = 0.0d0

        ELSE IF(SRMODE.EQ.1) THEN     ! DiFrancesco-Noble SR model
          IUP   = ALPHA(12)*Y(4) *(CA12M-Y(12))
          ITRAN = ALPHA(13)*Y(14)*(Y(12)-Y(13))

C ***     When rmode is zero repriming is voltage dependent

          IF(RMODE.EQ.0) THEN
            F(14) = SPEED(14)*(ALPHA(14)*(1.0d0-Y(14))-BETA(14)*Y(14))
          ELSE
            Y(14) = 1.0d0
            F(14) = 0.0d0
          ENDIF
          Z4 = 1.0d0
          Z5 = 1.0d0
          NNREL=INT(NREL+0.1d0) !CMISS addition 21/3/89
          DO L = 1,NNREL
            Z5 = Z5*Y(4)
            Z4 = Z4*KMCA
          ENDDO
          IREL  = BETA(13)*Y(13)*Z5/(Z5+Z4)
          F(12) = (IUP -ITRAN)/(2.0d0*V12F)
          F(13) = (ITRAN-IREL)/(2.0d0*V13F)
          DO L = 26,30
            F(L) = 0.0d0
            Y(L) = 0.0d0
          ENDDO

        ELSE IF(SRMODE.EQ.2) THEN       ! Hilgemann formulation
          Z3 = DEXP(0.08d0*(Y(1)-40.0d0))    ! Voltage dependence of release

          Z1 = Y(4)/(Y(4)+0.0005d0)
          Z1 = Z1*Z1                    ! Regulatory Ca binding site
          Z4 = BETA(26)*Z1+ALPHA(26)*Z3 ! Activation rate
          Z2 = BETA(27)*Z1+ALPHA(27)    ! Inactivation rate

          Y(26) = 1.0d0-Y(28)-Y(27)
          F(26) = Y(28)*BETA(28)-Y(26)*Z4    ! Precursor fraction
          F(27) = Y(26)*Z4-Y(27)*Z2          ! Activator fraction
          F(28) = Y(27)*Z2-Y(28)*BETA(28)    ! Product   fraction
          BETA(13) = Y(27)/(Y(27)+0.25d0)
          BETA(13) = BETA(13)*BETA(13)       ! Open rel channel fraction

          ITRAN = ALPHA(13)*(Y(12)-Y(13))    ! Change in uptake store

C ***     SR PUMP EQUATIONS
C ***     NOTE that all concentration changes to and from the cytosol
C ***     are calculated as concentration changes with respect to the
C ***     cytosol, and related by fractional volumes to the changes of
C ***     other states

          Z1 = KCYCA*KXCS/KSRCA                 ! K1 in paper
          Z2 = Y(4)+Y(12)*Z1+KCYCA*KXCS+KCYCA   ! K2 in paper
          FCYT = Y(4)/Z2                        ! Fr uptake Ca sites
          FSR  = Y(12)*Z1/Z2                    ! Fr back SR sites
C dpn 17/02/98          FCYTFREE = KCYCA/Z2                   ! Used for display
C dpn 17/02/98          FSRFREE  = KCYCA*KXCS/Z2

          IUP = FCYT*ALPHA(12)-FSR*BETA(12)     ! SR Ca uptake

          F(12) = IUP*VI/SRVOL-ITRAN            ! Uptake store
          IREL  = (BETA(13)*KMCA+SRleak)*Y(13)  ! Conc change of rel st

          F(13) = ITRAN*V12/V13-IREL            ! Release store

C ***     Note: W(8)=10 =0.2/0.02=fsrrel/fsrup -See Hilgemann PCIN1.PAS

        ENDIF

        IMCA = ICACA+IBCA+ICA2+ICA3-2.0d0*INACA/(NNACA-2.0d0)

        IF(SRMODE.LT.2) THEN     ! Pre-Hilgemann formulations
          IF(CABUFF) THEN
            F(4) = 0.0d0
          ELSE
            F(4) = (-IMCA+IREL-IUP)/(2.0d0*VIF)
            IF(BMODE.GT.0) THEN

C ***         When BMODE = 1 use Calgary buffer equations to subtract
C ***         calcium taken up by buffers and to adjust calcium space

              Z1 = NCALMOD*CALMOD*F(20)+NCTROP*CTROP*F(21)
              Z1 = Z1+NMTROP*MTROP*F(23)
              F(4) = F(4)/(CAVOL*BUFVOL)
              F(4) = F(4)-Z1
            ENDIF
          ENDIF
        ELSE                     ! Hilgemann formulation
          IF(SLPUMPMODE) THEN
C dpn 17/02/98            DCNP = IMCA
            IMCA = IMCA+(Y(4)/(Y(4)+KMSLPUMP))*KSLPUMP
C dpn 17/02/98            DCP  = IMCA
          ENDIF
          IF(BUFFAST) THEN
            F(4) = -IMCA/(2.0d0*VIF)-IUP+IREL*SRVOL*V13/(VI*V12)-F(21)-
     '        F(20)
          ELSE
            F(31)= -IMCA/(2.0d0*VIF)-IUP+IREL*SRVOL*V13/(VI*V12)
          ENDIF
          IF(CONTRACTMODE) THEN
            F(29) = (1.0d0-Y(29))*FMTROP*FCTROP*FCTROP*ALPHA(29)
     '        -Y(29)*BETA(29)
            F(30) = (1.0d0-Y(30))*Y(29)*ALPHA(30)-Y(30)*BETA(30)
          ELSE
            F(29) = 0.0d0
            F(30) = 0.0d0
          ENDIF
        ENDIF

        IF(MMODE.EQ.1) F(4) = F(4)-CAFMIT

      ELSE IF(CASS) THEN
C ***   If (Ca)i at steady state do not compute f(4)

        IF(NMODE.EQ.0) THEN
C ***     sinh function for INACA
C ***     NOTE: strongly recommended not to use this mode for INACA

          Z1 = (ICACA+IBCA+ICA2+ICA3)*(NNACA-2.0d0)/(2.0d0*KNACA)
          Z2 = (2.0d0*RTONF/(NNACA-2.0d0))*DLOG(Z1+SQRT((Z1*Z1)+1.0d0))
          ENA = -RTONF*DLOG(Y(2)/NAO) !?? is this correct PJH? 15Jan91
          Z1 = 0.5d0*(NNACA-2.0d0)*(-Y(1)+(NNACA*ENA/(NNACA-2.0d0))
     '      +Z2)/(0.5d0*RTONF)
          Y(4) = CAO*DEXP(-Z1)

        ELSE IF(NMODE.EQ.1) THEN
C ***     Full function for INACA

          Z1 = Y(1)*(NNACA-2.0d0)/(2.0d0*RTONF)
          Z2 = (ICACA+IBCA+ICA2+ICA3)*(NNACA-2.0d0)/(2.0d0*KNACA)
          Z4 = 1.0d0
          Z5 = 1.0d0
          DO L = 1,INT(NNACA+.01d0)  !CMISS 21/3/89
            Z4 = Z4*Y(2)
            Z5 = Z5*NAO
          ENDDO
          Z3 = Z4*CAO*DEXP(2.0d0*YNACA*Z1)
          Z3 = Z3-Z2*(1.0d0+DNACA*CAO*Z4)
          Y(4) = Z3/(Z5*(Z2*DNACA+DEXP(-2.0d0*(1.0d0-YNACA)*Z1)))
        ENDIF

        IF(Y(4).LT.CALIM) Y(4) = 0.99d0*CALIM
        F(4) = 0.0d0

C ***   Compute steady state calcium stores

        Z5 = 1.0d0
        Z4 = 1.0d0
        NNREL=INT(NREL+0.1d0) !CMISS addition 21/3/89
        DO L = 1,NNREL
          Z5 = Z5*Y(4)
          Z4 = Z4*KMCA
        ENDDO
        Z1 = (Z5+Z4)/Z5
        IF(RMODE.EQ.0) THEN
          Y(14) = ALPHA(14)/(ALPHA(14)+BETA(14))
        ELSE
          Y(14) = 1.0d0
        ENDIF
        Z2 = Y(4)*CA12M*(1.0d0+ALPHA(13)*Y(14)*Z1/BETA(13))
        Z3 = ALPHA(13)*Y(14)/ALPHA(12)
        Z4 = Z3+Y(4)*(1.0d0+ALPHA(13)*Y(14)*Z1/BETA(13))
        Y(12) = Z2/Z4
        Y(13) = Y(4)*ALPHA(12)*Z1*(CA12M-Y(12))/BETA(13)
        IF(DEBUG) WRITE(*,*) 'STEADY STATE LOOP'
        F(12) = 0.0d0
        F(13) = 0.0d0
        F(14) = 0.0d0
      ENDIF

C *** Compute extracellular calcium change

      IF(CAOMODE.EQ.1) THEN                       !1.2
        F(24) = 0.5d0*IMCA/VF-DIFFCA*(Y(24)-CAB)
        CAO   = Y(24)
      ELSE
        F(24) = 0.0d0                              !1.2
      ENDIF

C *** Compute change in intracellular potassium

      F(11) = -IMK/VIF

C *** Compute Ca & K changes due to perfusion

      IF(SPACE.EQ.5) THEN
        F(4)  = F(4) +(CAPE-Y(4))/TCAP
        F(11) = F(11)+(KPE-Y(11))/TKP
      ENDIF

      RETURN
      END


      SUBROUTINE DFN_CURRENTS(Y,F)

C#### Subroutine: DFN_CURRENTS
C###  Description:
C###    Computes ionic currents for HEART
C###    This version includes the options for Hilgemann's formulation of
C###    ITO (version 2.0) Anticipates Hilgemann Ca inactivation.

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs001.cmn'
      INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs004.cmn'
!     Parameter List
      REAL*8 F(*),Y(*) !,T
!     Common blocks
      REAL*8                    IIto,IICaCa,IICaK,IICaNa,IIK,IINa,
     '  IIFK,IIFNA
      COMMON /DFN_MAX_CURRENTS/ IIto,IICaCa,IICaK,IICaNa,IIK,IINa,
     '  IIFK,IIFNA
!     Local Variables
      INTEGER L
      REAL*8 E0,ECA,EK,EKC(0:11),EMH,ENA,ENACA,
     '  IBKC(0:11),ICAKC(0:11),IFKC(0:11),IFNAC(0:11),
     '  IK1C(0:11),IKC(0:11),IMKC(0:11),IPC(0:11),
     '  ITOC(0:11),ITOT,
     '  Z2,Z3,Z4,Z5
C dpn 07/05/98 unused real*8's IACHC(0:11),ISI,CACH(10),ECH(10),IMKSAV(0:11),IMNA,IREL,ITRAN,IUP,KCH(10),NACH(10),PUMPCH(10),SAVEN0(33),SAVEN1(33),SAVEN2(33),SAVEN3(33),SAVY(33),TCH(10),YRES(33),YSAV(0:11),
c dpn 07/05/98 unused      LOGICAL CONTINUE

!      write(*,'('' >>>call dfn_currents'')')

      IIK = 0.0d0
      IICaCa = 0.0d0
      IICaK = 0.0d0
      IICaNa = 0.0d0
      IIto = 0.0d0
      IINa = 0.0d0
      IIFK = 0.0d0
      IIFNA = 0.0d0

      IF(CAMODE.EQ.8) THEN
        CACHOFF = Y(4)/(KCACHOFF+Y(4))
        CACHON  = (1.0d0-Y(8))*(1.0d0-CACHOFF)
      ELSE
        CACHON  = Y(8)
      ENDIF

C *** Assume 12% K leak in m3h channel

      ENA = -RTONF*DLOG(Y(2)/NAO)
      IF(SPACE.GT.2) THEN
        Z1 = Y(3)
      ELSE
        Z1 = K
      ENDIF
      EMH = RTONF*DLOG((NAO+PNAK*Z1)/(Y(2)+PNAK*Y(11)))
C *** DPN 29 April 1998 - grab max INa for plotting
c      INA = GNA*Y(9)*Y(10)*Y(10)*Y(10)*(Y(1)-EMH)
      IINa = GNA*(Y(1)-EMH)
      INA = Y(9)*Y(10)*Y(10)*Y(10)*IINa ! h * m*m*m * INa(max)

C *** Background inward current. Note that the background channel may
C *** either have separate values for GBNA & GBK or one may assume 
C**** equal Na & K conductance (set GB only) as in Colqhoun,Neher,
C**** Reuter & Stevens.

      IBNA = GBNA*(Y(1)-ENA)

C *** Calculate Na Ca exchange current.
C *** For details of formulation of equations for inaca see Mullins,L.
C *** 1977  J.gen.physiol. 70, 681-695
C *** 1981  Ion Transport in Heart. Raven press and
C *** Noble, D. (1986) In Cardiac Muscle: Regulation of Exitation
C *** and Contraction ( Ed R.D. Nathan), Academic Press.
C *** Note that the procedure will hold inaca constant if ivplot is not
C *** instantaneous to eliminate large transient inaca at very negative
C *** potentials

      IF(.NOT.(MODE.LT.7.AND.PLOTT.AND.PMODE.GT.1)) THEN
        IF(Y(4).LT.1.d-8) Y(4) = 1.d-8
        ECA = -0.5d0*RTONF*DLOG(Y(4)/CAO)
        IF(Y(2).LT.1.d-5) Y(2) = 1.d-5
        IF(INT(NNACA+.01d0).EQ.2) THEN
          INACA = 0.0d0
        ELSE
          ENACA = (NNACA*ENA-2.0d0*ECA)/(NNACA-2.0d0)
          Z1 = RTONF*2.0d0/(NNACA-2.0d0)

          IF(NMODE.EQ.0) THEN
C ***       When NMODE is 0 use simple sinh formulation for inaca
C ***       Strongly recommended not to use this mode

            INACA = KNACA*(DEXP((Y(1)-ENACA)/Z1)-DEXP(-(Y(1)-ENACA)
     '        /Z1))/2.0d0

          ELSE IF(NMODE.EQ.1) THEN
C ***       When NMODE is 1 use Ca and Na activated inaca

            Z4 = 1.0d0
            Z5 = 1.0d0
            DO L = 1,INT(NNACA+.01d0)   !CMISS 21/3/89
              Z4 = Z4*NAO
              Z5 = Z5*Y(2)
            ENDDO
            Z2 = 1.0d0+DNACA*(Y(4)*Z4+CAO*Z5)
            Z3 = Z5*CAO*DEXP(2.0d0*YNACA*Y(1)/Z1)
            Z3 = Z3-Z4*Y(4)*DEXP(-2.0d0*(1.0d0-YNACA)*Y(1)/Z1)
            INACA = KNACA*Z3/Z2
          ENDIF
          IF(DABS(INACA).GT.INACAM) THEN
            IF(INACA.GT.0.0d0) THEN
              INACA =  INACAM
            ELSE
              INACA = -INACAM
            ENDIF
          ENDIF
          IF(KNACA.LT.0.0d0) INACA = 0.0d0
        ENDIF
      ENDIF

C *** Background inward calcium current

      IBCA = GBCA*(Y(1)-ECA)

      IF(SPACE.GT.2) THEN
C ***   Compute K dependent currents for 3 compartment model and for
C ***   non-restricted ecs

        IF(Y(3).LT.1.d-5) Y(3) = 1.d-5
        EK = RTONF*DLOG(Y(3)/Y(11))

C ***   IK1 rectifier described by blocking particle model with K activation.
C ***   K activation uses Michaelis Menten - cf Sakmann & Trube

        E0 = Y(1)-EK+10.0d0-SHIFTK1
        IK1 = GK1*(Y(3)/(KMK1+Y(3)))*(Y(1)-EK)/(1.0d0+DEXP(E0/
     '    (0.5d0*RTONF)))

        IF(TOMODE.EQ.0.OR.TOMODE.EQ.1) THEN
C ***     When TOMODE.EQ.0,1 ito described by outward rectifier with
C ***     instantaneous voltage activation

          E0 = Y(1)+10.0d0-SHIFTTO
          IF(DABS(E0).LT.1.d-4) THEN
            ITO = 1.4d0*(0.2d0+(Y(3)/(KMTO+Y(3))))
          ELSE
            ITO = 0.28d0*(0.2d0+(Y(3)/(KMTO+Y(3))))*E0/(1.0d0-
     '        DEXP(-0.2d0*E0))
          ENDIF
          ITO = GTO*ITO*(Y(11)*DEXP(0.02d0*Y(1))-Y(3)*DEXP(-0.02d0*
     '      Y(1)))
C ***     DPN 1/4/98 - grab the max (steady-state??) current
          IIto = ITO
          IF(TOMODE.EQ.1) ITO = Y(18)*ITO

        ELSE IF(TOMODE.EQ.2) THEN
C ***     When TOMODE=2 use full equations

C ***     DPN 1/4/98 - grab the max (steady-state??) current
C          ITO = Y(18)*Y(19)*0.28d0*(0.2d0+(Y(3)/(KMTO+Y(3))))
C          ITO = GTO*ITO*(Y(11)*DEXP(0.02d0*Y(1))-Y(3)*DEXP(-0.02d0*
C     '      Y(1)))
          IIto = 0.28d0*(0.2d0+(Y(3)/(KMTO+Y(3))))
          IIto = GTO*ITO*(Y(11)*DEXP(0.02d0*Y(1))-Y(3)*DEXP(-0.02d0*
     '      Y(1)))
          ITO = Y(18)*Y(19)*IIto

        ELSE IF(TOMODE.EQ.3.OR.TOMODE.EQ.4) THEN
C ***     When TOMODE=3,4 use Hilgemann formulation

          E0 = Y(1)-SHIFTTO
          IF(TOMODE.EQ.4) THEN
            F(17) = ((1.0d0/(1.0d0+DEXP(-(Y(1)+35.0d0)/5.0d0)))-
     '        Y(17))*ALPHA(17)
          ELSE
            F(17) = 0.0d0
            Y(17) = (1.0d0/(1.0d0+DEXP(-(Y(1)+35.0d0)/5.0d0)))
          ENDIF
C ???     DPN 1/4/98 - grab the max (steady-state??) current  ???
C          ITO = GTO*Y(17)*(Y(1)-EK)/(1.0d0+DEXP(E0/25.0d0))
           IIto = GTO*(Y(1)-EK)/(1.0d0+DEXP(E0/25.0d0))
           ITO = IIto * Y(17)
        ENDIF

C ***   IK uses rate theory inward rectifier, no -ve slope, no xover

C ***   dpn 2/4/98 - grab max IK
c        IK = Y(6)*IKM*(Y(11)-Y(3)*DEXP(-Y(1)/RTONF))/140.0d0
        IIK = IKM*(Y(11)-Y(3)*DEXP(-Y(1)/RTONF))/140.0d0
        IK  = Y(6)*IIK ! x * IIK
        IF(KMODE.EQ.3) IK = Y(6)*IK

C ***   Version 1.3 uses Acetyl-choline activated channel with i(V)
C ***   relation based on IK fully-activated relation

        IACH = IACHM*(Y(11)-Y(3)*DEXP(-Y(1)/RTONF))/140.0d0

C ***   IF(based on DiFrancesco (1981,1982) - K activation with
C ***   linear K and Na components

C ***   DPN 29 April 1998 - rearrange to get IIFK and IIFNA
c        IFK  = Y(5)*GFK *(Y(3)/(KMF+Y(3)))*(Y(1)-EK)
c        IFNA = Y(5)*GFNA*(Y(3)/(KMF+Y(3)))*(Y(1)-ENA)
        IIFK  = GFK *(Y(3)/(KMF+Y(3)))*(Y(1)-EK)
        IIFNA = GFNA*(Y(3)/(KMF+Y(3)))*(Y(1)-ENA)
        IFK  = Y(5)*IIFK
        IFNA = Y(5)*IIFNA

C ***   K component of ICA assumes 1% K permeability

        Z1 = PCAK*PCA*(Y(1)-VSURFCA)/(RTONF*(1.0d0-DEXP(-(Y(1)-VSURFCA)/
     '    RTONF)))
C ***   DPN 1/4/98 - grab max (steady-state ??) ICaK
c        ICAK = Y(7)*CACHON*Z1*(Y(11)*DEXP(VSURFCA/RTONF)
c     '    -Y(3)*DEXP(-(Y(1)-VSURFCA)/RTONF))
        IICaK = Z1*(Y(11)*DEXP(VSURFCA/RTONF)
     '    -Y(3)*DEXP(-(Y(1)-VSURFCA)/RTONF))
        ICAK = Y(7) * CACHON * IICaK !cachon = f, y(7) = d
        IF((CAMODE.EQ.2).OR.(CAMODE.EQ.4).OR.(CAMODE.EQ.5).OR.
     '    (CAMODE.EQ.6)) ICAK = Y(15)*ICAK

C ***   NaK pump current activated by Nai and Kc

        IP = PUMP*(Y(2)/(KMNA+Y(2)))*Y(3)/(KM+Y(3))

C ***   K contribution to background channel

        IBK = GBK*(Y(1)-EK)

C ***   Compute total K flux as a current.
C ***   Note that version 2.0 includes ITO in this sum

        IMK = IK+IK1+IFK+ICAK+IBK+ITO-IP/(NNAK-1.0d0)

      ELSE IF(SPACE.LT.3) THEN
C ***   All K dependent currents computed as function of (K)c and of
C ***   radial distance for cylinder or sphere.
C ***   current functions otherwise same as in previous section

        DO L = 0,DEPTH
          IF(KCE(L).LT.1.d-4) KCE(L) = IDINT(1.d-4) !1.d-4
          EKC(L) = RTONF*DLOG(KCE(L)/Y(11))
          E0 = Y(1)-EKC(L)+10.0d0-SHIFTK1
          IK1C(L) = GK1*(KCE(L)/(KMK1+KCE(L)))*(Y(1)-EKC(L))
          IK1C(L) = IK1C(L)/(1.0d0+DEXP(E0/(0.5d0*RTONF)))
          E0 = Y(1)+10.0d0-SHIFTTO
          IF(DABS(E0).LT.0.001d0) THEN
            ITOC(L) = 5.0d0*(0.2d0+(KCE(L)/(KMTO+KCE(L))))
          ELSE
            ITOC(L) = (0.2d0+(KCE(L)/(KMTO+KCE(L))))*E0/(1.0d0
     '        -DEXP(-0.2d0*E0))
          ENDIF
          ITOC(L)  = 0.28d0*GTO*ITOC(L)*(Y(11)*DEXP(0.02d0*Y(1))
     '      -KCE(L)*DEXP(-0.02d0*Y(1)))
          IF(TOMODE.NE.0) ITOC(L) = Y(18)*ITOC(L)
C ***     DPN 2/4/98 - rearrange to get max IK
c          IKC(L)   = Y(6)*IKM*(Y(11)-KCE(L)*DEXP(-Y(1)/RTONF))/140.0d0
c          IF(KMODE.EQ.3) IKC(L) = Y(6)*IKC(L)
          IKC(L)   = IKM*(Y(11)-KCE(L)*DEXP(-Y(1)/RTONF))/140.0d0

C dpn 17/02/98          IACHC(L) = IACHM*(Y(11)-KCE(L)*DEXP(-Y(1)/RTONF))/140.0d0   !Version 1.3
C ***     DPN 29 April 1998 - rearrange to get IIFK and IIFNA
c          IFKC(L)  = Y(5)*GFK*(KCE(L)/(KMF+KCE(L)))*(Y(1)-EKC(L))
c          IFNAC(L) = Y(5)*GFNA*(KCE(L)/(KMF+KCE(L)))*(Y(1)-ENA)
          IFKC(L)  = GFK*(KCE(L)/(KMF+KCE(L)))*(Y(1)-EKC(L))
          IFNAC(L) = GFNA*(KCE(L)/(KMF+KCE(L)))*(Y(1)-ENA)
          IPC(L) = PUMP*(Y(2)/(KMNA+Y(2)))*KCE(L)/(KM+KCE(L))
          Z1 = PCAK*PCA*(Y(1)-VSURFCA)/(RTONF*(1.0d0-DEXP(-(Y(1)-
     '      VSURFCA)/RTONF)))
          ICAKC(L) = Z1*(Y(11)*DEXP(VSURFCA/RTONF)
     '      -KCE(L)*DEXP(-(Y(1)-VSURFCA)/RTONF))
c dpn 1/4/98          ICAKC(L) = Y(7)*CACHON*ICAKC(L)
c dpn 1/4/98          IF((CAMODE.EQ.2).OR.(CAMODE.EQ.4).OR.(CAMODE.EQ.5).OR.
c     '      (CAMODE.EQ.6)) ICAKC(L) = Y(15)*ICAKC(L)
          IBKC(L) = GBK*(Y(1)-EKC(L))
          IF((CAMODE.EQ.2).OR.(CAMODE.EQ.4).OR.(CAMODE.EQ.5).OR.
     '      (CAMODE.EQ.6)) THEN
            IF(KMODE.EQ.3) THEN
              IMKC(L) = Y(6)*Y(6)*IKC(L)+IK1C(L)+Y(5)*IFKC(L)+
     '          Y(7)*Y(15)*ICAKC(L)+IBKC(L)-IPC(L)/(NNAK-1.0d0)
            ELSE
              IMKC(L) = Y(6)*IKC(L)+IK1C(L)+Y(5)*IFKC(L)+Y(7)*Y(15)*
     '          ICAKC(L)+IBKC(L)-IPC(L)/(NNAK-1.0d0)
            ENDIF
          ELSE
            IF(KMODE.EQ.3) THEN
              IMKC(L) = Y(6)*Y(6)*IKC(L)+IK1C(L)+Y(5)*IFKC(L)+
     '          Y(7)*ICAKC(L)+IBKC(L)-IPC(L)/(NNAK-1.0d0)
            ELSE
              IMKC(L) = Y(6)*IKC(L)+IK1C(L)+Y(5)*IFKC(L)+Y(7)*ICAKC(L)+
     '          IBKC(L)-IPC(L)/(NNAK-1.0d0)
            ENDIF
          ENDIF
        ENDDO
        K   = 0.0d0
        IK1 = 0.0d0
        IK  = 0.0d0
        IFK = 0.0d0
        IIFK = IFK
        IBK = 0.0d0
        IFNA= 0.0d0
        IIFNA = IFNA
        IP  = 0.0d0
        IMK = 0.0d0
        ICAK= 0.0d0
C ***   dpn 1/4/98 - grab max ICaK
        IICaK = ICAK
        ITO = 0.0d0
        IACH= 0.0d0
      ENDIF

C *** Total K dependent currents obtained by integrating radially

      IF(SPACE.EQ.1) THEN
C ***   Cylindrical case

        DO L = 1,DEPTH
          Z1 = (DBLE(L)-0.5d0)/DBLE(DEPTH*DEPTH)
          IK1 = IK1+(IK1C(L-1)+IK1C(L))*Z1
          ITO = ITO+(ITOC(L-1)+ITOC(L))*Z1
C ***     dpn 2/4/98 - grab max IK
c          IK = IK+(IKC(L-1)+IKC(L))*Z1
          IIK = IIK+(IKC(L-1)+IKC(L))*Z1
          IF(KMODE.EQ.3) THEN
            IK = IK + (Y(6)*Y(6)*IKC(L-1)+Y(6)*Y(6)*IKC(L))*Z1
          ELSE
            IK = IK + (Y(6)*IKC(L-1)+Y(6)*IKC(L))*Z1
          ENDIF
C ***     Version 1.3 --- Set IACH if needed

C ***     DPN 29 April 1998 - rearrange to get IIFK and IIFNA
c          IFK = IFK+(IFKC(L-1)+IFKC(L))*Z1
c          IFNA = IFNA+(IFNAC(L-1)+IFNAC(L))*Z1
          IIFK = IIFK+(IFKC(L-1)+IFKC(L))*Z1
          IIFNA = IIFNA+(IFNAC(L-1)+IFNAC(L))*Z1
          IFK = IFK+(Y(5)*IFKC(L-1)+Y(5)*IFKC(L))*Z1
          IFNA = IFNA+(Y(5)*IFNAC(L-1)+Y(5)*IFNAC(L))*Z1
          IP = IP+(IPC(L-1)+IPC(L))*Z1
C DPN 1/4/98 - rearrange to get IICaK
          IF((CAMODE.EQ.2).OR.(CAMODE.EQ.4).OR.(CAMODE.EQ.5).OR.
     '      (CAMODE.EQ.6)) THEN
            ICAK = ICAK+(Y(7)*Y(15)*ICAKC(L-1)+Y(7)*Y(15)*ICAKC(L))*Z1
          ELSE
            ICAK = ICAK+(Y(7)*ICAKC(L-1)+Y(7)*ICAKC(L))*Z1
          ENDIF
          IICaK = IICaK + (ICAKC(L-1)+ICAKC(L))*Z1
c          ICAK = ICAK+(ICAKC(L-1)+ICAKC(L))*Z1
          IBK = IBK+(IBKC(L-1)+IBKC(L))*Z1
          IMK = IMK+(IMKC(L-1)+IMKC(L))*Z1
          K = K+(KC(L-1)+KC(L))*Z1
        ENDDO

      ELSE IF(SPACE.EQ.2) THEN
C ***   Spherical case

        DO L = 1,DEPTH
          Z1 = DBLE(L*L*L-(L-1)*(L-1)*(L-1))/DBLE(DEPTH*DEPTH*DEPTH)
          IK1 = IK1+0.5d0*(IK1C(L-1)+IK1C(L))*Z1
          ITO = ITO+0.5d0*(ITOC(L-1)+ITOC(L))*Z1
C ***     dpn 2/4/98 - grab max IK
c          IK = IK+0.5d0*(IKC(L-1)+IKC(L))*Z1
          IIK = IIK+0.5d0*(IKC(L-1)+IKC(L))*Z1
          IF(KMODE.EQ.3) THEN
            IK = IK+0.5d0*(Y(6)*Y(6)*IKC(L-1)+Y(6)*Y(6)*IKC(L))*Z1
          ELSE
            IK = IK+0.5d0*(Y(6)*IKC(L-1)+Y(6)*IKC(L))*Z1
          ENDIF
C ***     Version 1.3  ------- Set IACH if needed

C ***     DPN 29 April 1998 - rearrange to get IIFK and IIFNA
c          IFK = IFK+0.5d0*(IFKC(L-1)+IFKC(L))*Z1
c          IFNA = IFNA+0.5d0*(IFNAC(L-1)+IFNAC(L))*Z1
          IIFK = IFK+0.5d0*(IFKC(L-1)+IFKC(L))*Z1
          IIFNA = IFNA+0.5d0*(IFNAC(L-1)+IFNAC(L))*Z1
          IFK = IFK+0.5d0*(Y(5)*IFKC(L-1)+Y(5)*IFKC(L))*Z1
          IFNA = IFNA+0.5d0*(Y(5)*IFNAC(L-1)+Y(5)*IFNAC(L))*Z1
          IP = IP+0.5d0*(IPC(L-1)+IPC(L))*Z1
C DPN 1/4/98
C          ICAK = ICAK+0.5d0*(ICAKC(L-1)+ICAKC(L))*Z1
          IF((CAMODE.EQ.2).OR.(CAMODE.EQ.4).OR.(CAMODE.EQ.5).OR.
     '      (CAMODE.EQ.6)) THEN
            ICAK = ICAK+0.5d0*(Y(7)*Y(15)*ICAKC(L-1)+
     '        Y(7)*Y(15)*ICAKC(L))*Z1
          ELSE
            ICAK = ICAK+0.5d0*(Y(7)*ICAKC(L-1)+
     '        Y(7)*ICAKC(L))*Z1
          ENDIF
          IICAK = IICAK+0.5d0*(ICAKC(L-1)+ICAKC(L))*Z1
          IBK = IBK+0.5d0*(IBKC(L-1)+IBKC(L))*Z1
          IMK = IMK+0.5d0*(IMKC(L-1)+IMKC(L))*Z1
          K = K+0.5d0*(KC(L-1)+KC(L))*Z1
        ENDDO
      ENDIF

C *** For details of formulation of components of ICA see
C *** Reuter & Scholz, J.Physiol,1977,264,17-47
C *** The formulation here uses the constant field
C *** current voltage relations from that paper but does not
C *** use their (incorrect) equation for the channel
C *** reversal potential. See Campbell, Giles, Hume & Noble,
C *** J.Physiol (1987). The reversal potential is computed in
C *** procedure CAREV.
! DPN 19/02/98 CAREV unused ???
C**
C *** The correspondence with patch clamp work is that ICA
C *** corresponds best to the L current, ICA2 corresponds to
C *** the T current and ICA3 corresponds to the ICas of
C *** Lee et al, Proc Roy Soc 1984

      Z1 = (Y(1)-VSURFCA)/(RTONF*(1.0d0-DEXP(-(Y(1)-VSURFCA)/
     '  (0.5d0*RTONF))))
      Z1 = Z1*(Y(4)*DEXP(VSURFCA/(0.5d0*RTONF))
     '  -CAO*DEXP(-(Y(1)-VSURFCA)/(0.5d0*RTONF)))
C *** dpn 1/4/98 - grab max ICaCa
c      ICACA = 4.0d0*PCA*Y(7)*CACHON*Z1
      IICaCa = 4.0d0*PCA*Z1
      ICACA = Y(7) * CACHON * IICaCa
      IF(PCA2.NE.0.0d0) THEN
        ICA2 = 4.0d0*PCA2*Y(32)*Y(33)*Z1
      ELSE
        ICA2 = 0.0d0
      ENDIF
      IF(PCA3.NE.0.0d0) THEN
        ICA3 = 4.0d0*PCA3*Y(16)*Y(17)*Z1
      ELSE
        ICA3 = 0.0d0
      ENDIF
      Z1 = PCAK*PCA*(Y(1)-VSURFCA)/(RTONF*(1.0d0-DEXP(-(Y(1)-VSURFCA)/
     '  RTONF)))
C *** dpn 1/4/98 - grab max ICaNa
c      ICANA = Y(7)*CACHON*Z1*(Y(2)*DEXP(VSURFCA/RTONF)
c     '  -NAO*DEXP(-(Y(1)-VSURFCA)/RTONF))
      IICaNa = Z1*(Y(2)*DEXP(VSURFCA/RTONF)
     '  -NAO*DEXP(-(Y(1)-VSURFCA)/RTONF))
      ICANA = Y(7) * CACHON * IICaNa
      IF((CAMODE.EQ.2).OR.(CAMODE.EQ.4).
     'OR.(CAMODE.EQ.5).OR.(CAMODE.EQ.6)) THEN
        ICANA = Y(15)*ICANA
        ICACA = Y(15)*ICACA
      ENDIF
      IF(CANMODE.EQ.0) THEN
        ICANA = 0.0d0
        IICaNa = 0.0d0
      ENDIF
      ICA = ICACA+ICANA+ICAK

C *** Compute activation of Ca sensitive currents (other than inaca)
C *** each current is computed according to equation
C**
C *** i  =  i ( mi  +  (cai/(cai + cact)))
C**
C *** where mi is minimum value of current, expressed as a fraction
C *** of the value in amode 0 and cact is the calcium concentration
C *** for half-activation

      IF((AMODE.EQ.1).OR.(AMODE.EQ.9)) THEN
        IK1 = IK1*(MIK1+(Y(4)/(Y(4)+CACT(1))))
      ENDIF
      IF((AMODE.EQ.2).OR.(AMODE.EQ.9)) THEN
        IK = IK*(MIK+(Y(4)/(Y(4)+CACT(2))))
      ENDIF
      IF((AMODE.EQ.3).OR.(AMODE.EQ.9)) THEN
        IBK  = IBK *(MIB+(Y(4)/(Y(4)+CACT(3))))
        IBNA = IBNA*(MIB+(Y(4)/(Y(4)+CACT(3))))
      ENDIF
      IF((AMODE.EQ.4).OR.(AMODE.EQ.9)) THEN
        ITO = ITO*(MITO+(Y(4)/(Y(4)+CACT(4))))
      ENDIF
      IF((AMODE.EQ.5).OR.(AMODE.EQ.9)) THEN
        INAK = 0.5d0*(Y(4)/(Y(4)+CACT(3)))*(GNAK*(Y(1)-ENA)+GNAK*(Y(1)-
     '    EK))
      ENDIF

C *** Total current
C *** DPN 15/04/98 - adding switches to each current
      ITOT = ISWTCH(16)*INA+ISWTCH(5)*ICA+ISWTCH(6)*ICA2+ISWTCH(7)*ICA3
     '  +ISWTCH(13)*IK+ISWTCH(12)*IFNA+ISWTCH(11)*IFK+ISWTCH(19)*IP
     '  +ISWTCH(4)*IBNA+ISWTCH(3)*IBK+ISWTCH(2)*IBCA+ISWTCH(14)*IK1
     '  +ISWTCH(20)*ITO+IPULSE+STIM+ISWTCH(17)*INACA+ISWTCH(18)*INAK
      ITOT = ITOT+ISWTCH(1)*IACH                    !Version 1.3
C dpn 17/02/98      ISI = ICA+ICA2+ICA3+INACA

C *** Set dV/dt = 0 to give voltage clamp (modes 2 - 8).
C *** In  mode  2 compute effect of series resistance and of
C *** external clamp circuit. In action potential mode set
C *** dV/dt = - total current/capacitance

      IF(CLAMP) THEN
        F(1) = 0.0d0
      ELSE
        F(1) = -CAP*ITOT
      ENDIF
      IF(MODE.EQ.2) THEN
        IF((RS.NE.0.0d0).AND.(AMP.EQ.0.0d0)) THEN
          F(1) = (EC-Y(1)-ITOT*RS)/(C*RS)
        ENDIF
        IF(AMP.NE.0.0d0) THEN
          F(1) = (-ITOT+(AMP*(EC-Y(1))/(RE+RS*(1.0d0+AMP))))/C
        ENDIF
      ENDIF

      RETURN
      END


      SUBROUTINE DFN_CURRENTS_PLOT(NT_DAT,POTENTIALS,CURRENT,
     '  RESULTS,RPARAM,RPARAM_2,IPARAM_1,IPARAM_2,LPARAM)

C#### Subroutine: DFN_CURRENTS_PLOT
C###  Description:
C###    <html><pre>
C###    Calculates the specified current for computing the 
C###    steady state current
C###    CURRENT:- 1 = IICa   - Ca cpt of I(CaL) (maximum)
C###              2 = IICaK  - K cpt of I(CaL)
C###              3 = IICaNa - Na cpt of I(CaL)
C###              4 = IICa + IICaK + IICaNa
C###              5 = IIto
C###              6 = IIK
C###              7 = IINa
C###              9 = IIFK
C###             10 = IIFNa
C###             11 = IIF (= IIFK + IIFNa)
C###    </pre></html>

      IMPLICIT NONE
!     Parameter List
      INTEGER NT_DAT,CURRENT,IPARAM_1(9),IPARAM_2(13)
      LOGICAL LPARAM(5)
      REAL POTENTIALS(*),RESULTS(*)
      REAL*8 RPARAM(21,9),RPARAM_2(9)
!     Common blocks
      REAL*8                    IIto,IICaCa,IICaK,IICaNa,IIK,IINa,
     '  IIFK,IIFNA
      COMMON /DFN_MAX_CURRENTS/ IIto,IICaCa,IICaK,IICaNa,IIK,IINa,
     '  IIFK,IIFNA
!     Local Variables
      INTEGER i
      REAL*8 Y(33),F(33) !,t
      
C *** initialise the model parameters
      CALL OXSINI(IPARAM_1,IPARAM_2,LPARAM,RPARAM,
     '  RPARAM_2,Y)
      CALL OXSPREP(Y)

c      t = 0.0d0

C *** Calculate the rate coefficients
      DO i=1,NT_DAT
        Y(1)  = DBLE(POTENTIALS(i)) ! V
c        CALL DFN_CURRENTS(t,Y,F)
        CALL DFN_CURRENTS(Y,F)
        IF (CURRENT.EQ.1) THEN
          RESULTS(i) = SNGL(IICaCa)
        ELSE IF (CURRENT.EQ.2) THEN
          RESULTS(i) = SNGL(IICaK)
        ELSE IF (CURRENT.EQ.3) THEN
          RESULTS(i) = SNGL(IICaNa)
        ELSE IF (CURRENT.EQ.4) THEN
          RESULTS(i) = SNGL(IICaCa + IICaK + IICaNa)
        ELSE IF (CURRENT.EQ.5) THEN
          RESULTS(i) = SNGL(IIto)
        ELSE IF (CURRENT.EQ.6) THEN
          RESULTS(i) = SNGL(IIK)
        ELSE IF (CURRENT.EQ.7) THEN
          RESULTS(i) = SNGL(IINa)
        ELSE IF (CURRENT.EQ.9) THEN
          RESULTS(i) = SNGL(IIFK)
        ELSE IF (CURRENT.EQ.10) THEN
          RESULTS(i) = SNGL(IIFNA)
        ELSE IF (CURRENT.EQ.11) THEN
          RESULTS(i) = SNGL(IIFNA + IIFK)
        ENDIF
      ENDDO
      RETURN
      END


      SUBROUTINE DFN_RATES(Y,F)

C#### Subroutine: DFN_RATES
C###  Description:
C###    <html><pre>
C###    Computes the rate coefficients for the gating variables for 
C###    HEART.
C###    This version (2.0) includes Hilgemann formulation of Ca current
C###    inactivation (series V and Ca instead of parallel formulation of
C###    the DiFrancesco-Noble model).
C###    Update 1.4 incorporated separate T & L channels for the "fast"
C###    calcium current.
C###    </pre></html>

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs001.cmn'
      INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs004.cmn'
!     Parameter List
      REAL*8 F(*),Y(*) !,T
!     Local Variables
      REAL*8 FSS,Z3!,Z2,Z4
c dpn 07/05/98 unused real*8's - CACH(10),E0,ECH(10),EKC(0:11),IACHC(0:11),IBKC(0:11),ICAKC(0:11),IFKC(0:11),IFNAC(0:11),IKC(0:11),IK1C(0:11),IMKC(0:11),IMKSAV(0:11),IMNA,IPC(0:11),IREL,ISI,ITOC(0:11),ITOT,ITRAN,IUP,KCH(10),NACH(10),PUMPCH(10),TCH(10),YSAV(0:11),
!      write(*,'('' >>>call dfn_rates'')')

      IF(Y(1).NE.ESAV) THEN
C *** DPN 19/03/1998 - move calculation of gating rate const.'s
c        CALL DFN_RATES_SUB(ALPHA,BETA,SHIFT,SPEED,Y(1),FSS)
        CALL DFN_RATES_SUB(ALPHA,BETA,SHIFT,Y(1),FSS)
C *** DPN 19/03/1998 - adjust beta(15) with [Ca]i
        BETA(15) = BETA(15)*Y(4)
      ENDIF    ! end of computation of gate rate coefficients


C *** Check for quasi-instantaneous & steady state ivplots.
C *** For slow clamps set gates to steady state values.
C *** In mode 2 use steady state ina and isi negative to -85
C *** (note: this speeds computation.  if you wish to compute
C *** inward tail currents negative to -85 remove third condition)

      IF(SLOW.OR.PLOTT) THEN
        IF((.NOT.PLOTT).OR.(PMODE.GT.2)) THEN
          Y(5) = ALPHA(5)/(ALPHA(5)+BETA(5))
          F(5) = 0.0d0
          Y(6) = ALPHA(6)/(ALPHA(6)+BETA(6))
          F(6) = 0.0d0
        ENDIF
        Y(7) = ALPHA(7)/(ALPHA(7)+BETA(7))
        F(7) = 0.0d0

C ***   In CAMODE 7 use steady state value FSS
        IF(CAMODE.NE.7) THEN
          Y(8) = ALPHA(8)/(ALPHA(8)+BETA(8))
        ELSE
          Y(8) = FSS
        ENDIF
        F(8) = 0.0d0

        Y(9)  = ALPHA(9)/(ALPHA(9)+BETA(9))
        F(9)  = 0.0d0
        Y(10) = ALPHA(10)/(ALPHA(10)+BETA(10))
        F(10) = 0.0d0
        IF( (CAMODE.EQ.2).OR.(CAMODE.EQ.4).OR.(CAMODE.EQ.5)
     '  .OR.(CAMODE.EQ.6)) THEN
          Y(15) = ALPHA(15)/(ALPHA(15)+BETA(15))
        ELSE
          Y(15) = 1.0d0
        ENDIF
        F(15) = 0.0d0
        IF(PCA3.NE.0) THEN
          Y(16) = ALPHA(16)/(ALPHA(16)+BETA(16))
          Y(17) = ALPHA(17)/(ALPHA(17)+BETA(17))
        ELSE
          Y(16) = 0.0d0
          Y(17) = 0.0d0
        ENDIF
        F(16) = 0.0d0
        F(17) = 0.0d0                  ! correction  2.0
        IF(PCA2.NE.0) THEN
          Y(32) = ALPHA(32)/(ALPHA(32)+BETA(32))
          Y(33) = ALPHA(33)/(ALPHA(33)+BETA(33))
        ELSE
          Y(32) = 0.0d0
          Y(33) = 0.0d0
        ENDIF
        F(32) = 0.0d0
        F(33) = 0.0d0                  ! correction  2.0
        IF(TOMODE.NE.0) THEN
          Y(18) = ALPHA(18)/(ALPHA(18)+BETA(18))
          IF(TOMODE.EQ.2) THEN
            Y(19) = ALPHA(19)/(ALPHA(19)+BETA(19))
          ELSE
            Y(19) = 1.0d0
          ENDIF
        ELSE
          Y(18) = 1.0d0
          Y(19) = 1.0d0
        ENDIF
        F(18) = 0.0d0
        F(19) = 0.0d0

      ELSE
C ***   When not in slow mode, compute gate rates etc
C ***   compute dy/dt, dx/dt

        F(5) = SPEED(5)*(ALPHA(5)-Y(5)*(ALPHA(5)+BETA(5)))
        F(6) = SPEED(6)*(ALPHA(6)-Y(6)*(ALPHA(6)+BETA(6)))

C ***   compute dr/dt and dq/dt

        IF((TOMODE.NE.0)) THEN
          F(18) = SPEED(18)*(ALPHA(18)-Y(18)*(ALPHA(18)+BETA(18)))
          IF(TOMODE.EQ.2) THEN
            F(19) = SPEED(19)*(ALPHA(19)-Y(19)*(ALPHA(19)+BETA(19)))
          ELSE
            F(19) = 0.0d0
            Y(19) = 1.0d0
          ENDIF
        ELSE
          F(18) = 0.0d0
          F(19) = 0.0d0
          Y(18) = 1.0d0
          Y(19) = 1.0d0
        ENDIF

C ***   compute dd/dt, df/dt, & dh/dt

        F(7) = SPEED(7)*(ALPHA(7)-Y(7)*(ALPHA(7)+BETA(7)))

        IF(CAMODE.LE.6) THEN
C ***     When CAMODE.LT.7 use DiFrancesco-Noble formulation

          F(8) = SPEED(8)*(ALPHA(8)-Y(8)*(ALPHA(8)+BETA(8)))

        ELSE IF(CAMODE.EQ.7) THEN
C ***     When CAMODE=7 use Randy's formulation

          F(8) = ALPHA(8)*(FSS-Y(8))

        ELSE IF(CAMODE.EQ.8) THEN
C ***     When CAMODE=8 use Hilgemann formulation

          Z1 = (1.0d0-Y(8))*(1.0d0-CACHOFF) ! -Ca/not V-inactivated
C dpn 17/02/98          Z2 = Y(8)*(1.0d0-CACHOFF)         ! -Ca/    V-inactivated
          Z3 = (1.0d0-Y(8))*CACHOFF         ! +Ca/not V-inactivated
C dpn 17/02/98          Z4 = Y(8)*CACHOFF                 ! +Ca/    V-inactivated
          F(8) = SPEED(8)*(120.0d0*Z3+Z1)*BETA(8)-Y(8)*ALPHA(8)

        ENDIF

        IF( (CAMODE.EQ.2).OR.(CAMODE.EQ.4).OR.(CAMODE.EQ.5)
     '  .OR.(CAMODE.EQ.6)) THEN
          F(15) = SPEED(15)*(ALPHA(15)-Y(15)*(ALPHA(15)+BETA(15)))
        ELSE
          F(15) = 0.0d0
        ENDIF

C ***   compute dd3/dt and df3/dt

        IF(PCA3.NE.0) THEN
          F(16) = SPEED(16)*(ALPHA(16)-Y(16)*(ALPHA(16)+BETA(16)))
          F(17) = SPEED(17)*(ALPHA(17)-Y(17)*(ALPHA(17)+BETA(17)))
        ELSE
          F(16) = 0.0d0
          F(17) = 0.0d0
        ENDIF

C ***   compute d and f changes for T channel

        IF(PCA2.NE.0) THEN
          F(32) = SPEED(32)*(ALPHA(32)-Y(32)*(ALPHA(32)+BETA(32)))
          F(33) = SPEED(33)*(ALPHA(33)-Y(33)*(ALPHA(33)+BETA(33)))
        ELSE
          F(32) = 0.0d0
          F(33) = 0.0d0
        ENDIF

C ***   test whether ina at steady state

        IF(GNASS) THEN
          Y(9) = ALPHA(9)/(ALPHA(9)+BETA(9))
          F(9) = 0.0d0
        ELSE
          F(9) = SPEED(9)*(ALPHA(9)-Y(9)*(ALPHA(9)+BETA(9)))
        ENDIF

C ***   compute dm/dt when fast is true, else use steady state value

        IF(FAST) THEN
          F(10) = SPEED(10)*(ALPHA(10)-Y(10)*(ALPHA(10)+BETA(10)))
        ELSE
          Y(10) = ALPHA(10)/(ALPHA(10)+BETA(10))
          F(10) = 0.0d0
        ENDIF

      ENDIF
c     write(*,'('' speed(7)='',e11.3,'' shift(7)='',e11.3,
c    '  '' alpha(7)='',e11.3,'' beta(7)='',e11.3,'' f(7)='',e11.3)')
c    '   speed(7),shift(7),alpha(7),beta(7),f(7)

      RETURN
      END


      SUBROUTINE DFN_RATES_SUB(ALPHA,BETA,SHIFT,POT,FSS)

C#### Subroutine: DFN_RATES_SUB
C###  Description:
C###    <html><pre>
C###    Computes the rate coefficients for the gating variables for 
C###    HEART.
C###    This version (2.0) includes Hilgemann formulation of Ca current
C###    inactivation (series V and Ca instead of parallel formulation of
C###    the DiFrancesco-Noble model).
C###    Update 1.4 incorporated separate T & L channels for the "fast"
C###    calcium current.
C###    ALPHA(5) = y gate (I(f))
C###      "   6  = x gate (I(K))
C###      "   7  = d gate (Ca activation)
C###      "   8  = f gate (Ca inactivation)
C###      "   9  = h gate (Na inactivation gate)
C###      "  10  = m gate (Na activation gate)
C###      "  14  = Ca store repriming
C###      "  15  = slow Ca inactivation
C###      "  16  = D3 (activation of I(Ca,3) =I(Ca(L)) ??)
C###      "  17  = F3 (inactivation of I(Ca,3))
C###      "  18  = I(to) inactivation
C###      "  19  = I(to) activation
C###      "  32  = activation of T channel Ca current
C###      "  33  = inactivation of T channel Ca current
C###    BETA( 1) = 
C###      "   2  =
C###      "   3  =
C###      "   4  =
C###      "   5  =
C###      "   6  =
C###      "   7  = Ca
C###      "   8  =
C###      "   9  =
C###      "  10  =
C###      "  11  =
C###      "  12  =
C###      "  13  =
C###      "  14  =
C###      "  15  =
C###    </pre></html>

      IMPLICIT NONE
      INCLUDE 'cell/deoxs00.cmn'
C      INCLUDE 'cell/oxs001.cmn'
      INCLUDE 'cell/oxs003.cmn'
!     Parameter List
      REAL*8 ALPHA(33),BETA(33),SHIFT(33),POT,FSS
!     SMAR009 23/12/98 ,SPEED(33)
!     Local Variables
      REAL*8 E0

!     write(*,'('' >>>call dfn_rates_sub'')')

C *** y rate equations as in MNT model with alpha/beta interchanged

      E0 = POT+52.0d0-SHIFT(5)
      ALPHA(5) = 0.05d0*DEXP(-0.067d0*E0)
      IF(DABS(E0).LT.1.d-5) THEN
        BETA(5) = 2.5d0
      ELSE
        BETA(5) = E0/(1.0d0-DEXP(-0.2d0*E0))
      ENDIF

      IF(KMODE.EQ.0) THEN
C *** x rate equations as in MNT model when kmode = 0

        E0 = POT+50.0d0-SHIFT(6)
        ALPHA(6) = 0.5d0*DEXP(0.0826d0*E0)/(1.0d0+DEXP(0.057d0*E0))
        E0 = POT+20.0d0-SHIFT(6)
        BETA(6) = 1.3d0*DEXP(-0.06d0*E0)/(1.0d0+DEXP(-0.04d0*E0))

      ELSE IF(KMODE.EQ.1) THEN
C *** x rates  based  on DiFrancesco,  Noma  & Trautwein  and Oxford
C *** sinus  results  when  kmode = 1

        E0 = POT+22.0d0-SHIFT(6)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(6) = 2.5d0
          BETA(6)  = 2.5d0
        ELSE
          ALPHA(6) = 0.5d0*E0/(1.0d0-DEXP(-E0/5.0d0))
          BETA(6)  = 0.357d0*E0/(DEXP(E0/7.0d0)-1.0d0)
        ENDIF

      ELSE IF(KMODE.EQ.2) THEN
C *** x rates slower in diastolic range if kmode = 2

        E0 = POT+22.0d0-SHIFT(6)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(6) = 2.5d0
          BETA(6)  = 2.5d0
        ELSE
          ALPHA(6) = 0.5d0*E0/(1.0d0-DEXP(-E0/5.0d0))
          BETA(6)  = 0.178d0*E0/(DEXP(E0/15.0d0)-1.0d0)
        ENDIF

      ELSE IF(KMODE.EQ.3) THEN
C *** Galveston rate functions used when kmode = 3

        E0 = POT+26.488d0-SHIFT(6)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(6) = 0.1072d0
        ELSE
          ALPHA(6) = 0.01344d0*E0/(1.0d0-DEXP(-0.124d0*E0))
        ENDIF
        BETA(6) = 0.2063d0*DEXP(-0.039268d0*E0)
      ENDIF


C *** Compute ca rates

      IF(CAMODE.EQ.1.OR.CAMODE.EQ.2.OR.CAMODE.EQ.5.OR.CAMODE.EQ.8)
     '  THEN
C *** If camode = 1, 2, 5 or 8 use 'Oxford' sinus d & f
C *** CAMODE 8 is the Hilgemann formulation

        E0 = POT+24.0d0-SHIFT(7)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(7) = 120.0d0
        ELSE
          ALPHA(7) = 30.0d0*E0/(1.0d0-DEXP(-E0/4.0d0))
        ENDIF
        IF(DABS(E0).LT.1.d-4) THEN
          BETA(7) = 120.0d0
        ELSE
          BETA(7) = 12.0d0*E0/(DEXP(E0/10.0d0)-1.0d0)
        ENDIF

      ELSE IF(CAMODE.EQ.3.OR.CAMODE.EQ.4.OR.CAMODE.EQ.6) THEN
C *** if camode = 3, 4 or 6 use Galveston d equations

        E0 = POT+12.07d0-SHIFT(7)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(7) = 10.7d0
        ELSE
          ALPHA(7) = 18.1d0*E0/(1.0d0-DEXP(-0.169d0*E0))
        ENDIF
        BETA(7) = 40.18d0*DEXP(-0.0459d0*E0)

      ELSE IF(CAMODE.EQ.7) THEN
C *** if CAMODE=7 use Calgary d equations

        E0 = POT+9.1d0-SHIFT(7)
        ALPHA(7) = 10.0d0*E0/(1.0d0-DEXP(-E0/8.23d0))
        BETA(7)  = ALPHA(7)*DEXP(-E0/8.23d0)
      ENDIF

      IF(CAMODE.LE.6) THEN
C *** In all camodes except 7 & 8 use 'oxford' f equations

        E0 = POT+34-SHIFT(8)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(8) = 25.0d0
        ELSE
          ALPHA(8) = 6.25d0*E0/(DEXP(E0/4.0d0)-1.0d0)
        ENDIF
        BETA(8) = 50.0d0/(1.0d0+DEXP(-E0/4.0d0))

      ELSE IF(CAMODE.EQ.7) THEN
C *** In CAMODE=7 use Randy's f formulation

        E0 = 0.03368d0*(POT+10.0d0)
        E0 = E0*E0
        ALPHA(8) = 19.6978d0*DEXP(-E0)+19.78848d0
        FSS = (1.0d0-0.01d0)/(1.0d0+DEXP(-((-28.05d0-POT)/8.58d0)))
        FSS = FSS+0.01d0+1.0d0/(1.0d0+DEXP((50.0d0-POT)/20.0d0))
        BETA(8) = 0.0d0

      ELSE IF(CAMODE.EQ.8) THEN
C *** In CAMODE=8 use Hilgemann formulation

        E0 = POT+34.0d0-SHIFT(8)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(8) = 25.0d0
        ELSE
          ALPHA(8) = 6.25d0*E0/(DEXP(E0/4.0d0)-1.0d0)
        ENDIF
        BETA(8) = 12.0d0/(1.0d0+DEXP(-(E0/4.0d0)))

C *** Note: rest of Hilgemann form left until computation of F(8)
      ENDIF

      IF((CAMODE.EQ.2).OR.(CAMODE.EQ.4)) THEN
C *** In camode 2 or 4 compute slow ca inactivation

        E0 = POT+34.0d0-SHIFT(15)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(15) = 2.5d0
        ELSE
          ALPHA(15) = 0.625d0*E0/(DEXP(E0/4.0d0)-1.0d0)
        ENDIF
        BETA(15) = 5.0d0/(1.0d0+DEXP(-E0/4.0d0))

      ELSE IF((CAMODE.EQ.5).OR.(CAMODE.EQ.6)) THEN
C **  In camode 5 & 6  inactivation is ca induced using equation:
C **
C **      df2/dt = alpha(15)(1 - f2)   -   beta(15)*cai*f2
C **
C **  The equations use the relations:
C **
C **      ALPHA(15) = 1/TAUF2
C **
C **  Where TAUF2 is the limiting recovery time constant when (Ca)i->0
C **
C **      BETA(15) = ALPHA(15)/KMINACT
C **
C **  where KMINACT is (Ca)i for half inactivation in steady state.
C **
C **  For convenience in programming the actual BETA(15) in the program
C **  is (Ca)i*BETA(15). This enables the same differential equation to
C **  be used later in the program regardless of the value of CAMODE

        BETA(15) = ALPHA(15)/KMINACT
      ENDIF

C *** ca store repriming equations based on those for f for voltage dep.

      E0 = POT+34.0d0-SHIFT(14)
      IF(DABS(E0).LT.1.d-4) THEN
        ALPHA(14) = 2.5d0
      ELSE
        ALPHA(14) = 0.625d0*E0/(DEXP(E0/4.0d0)-1.0d0)
      ENDIF
      BETA(14) = 5.0d0/(1.0d0+DEXP(-E0/4.0d0))

C *** NB  HILGEMANN ITO STILL TO BE ADDED !!!!!!!!!!!!!!!!!!!!!!
C *** Ito inactivation based on Dario's modification of Fozzard &
C *** Hiraoka (J.Physiol, 1973)

      IF(TOMODE.NE. 0) THEN
        E0 = POT-SHIFT(18)
        ALPHA(18) = 0.033d0*DEXP(-E0/17.0d0)
        BETA(18) = 33.0d0/(1.0d0+DEXP(-(E0+10.0d0)/8.0d0))

        IF(TOMODE.EQ.2) THEN
C ***   Ito activation based on MNT model equations

          E0 = POT-SHIFT(19)
          IF(DABS(E0).LT.1.d-4) THEN
            ALPHA(19) = 80.0d0
          ELSE
            ALPHA(19) = 8.0d0*E0/(1.0d0-DEXP(-0.1d0*E0))
          ENDIF
          BETA(19) = 80.0d0/(0.1d0+DEXP(0.0888d0*E0))
        ENDIF
      ENDIF

C *** Slow ca current: GP cell model

      IF(PCA3.NE.0) THEN
        E0 = POT+55.0d0-SHIFT(16)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(16) = 5.0d0
        ELSE
          ALPHA(16) = 1.25d0*E0/(1.0d0-DEXP(-E0/4.0d0))
        ENDIF
        IF(DABS(E0).LT.1.d-4) THEN
          BETA(16) = 5.0d0
        ELSE
          BETA(16) = 0.5d0*E0/(DEXP(E0/10.0d0)-1.0d0)
        ENDIF
        E0 = POT+45.0d0-SHIFT(17)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(17) = 0.84d0
        ELSE
          ALPHA(17) = 0.21d0*E0/(DEXP(E0/4.0d0)-1.0d0)
        ENDIF
        BETA(17) = 1.7d0/(1.0d0+DEXP(-E0/4.0d0))
      ENDIF

      IF(PCA2.NE.0) THEN
C *** Low threshold T channel
C *** provisionally, this channel is given the same equations as for
C *** the L channel (ICA). You should use SHIFT32, SHIFT33, SPEED32 and
C *** SPEED33 to shift the activation and inactivation curves and to
C *** increase the rates. When more information is available, these
C *** equations will be updated }

        E0 = POT+24.0d0-SHIFT(32)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(32) = 120.0d0
        ELSE
          ALPHA(32) = 30.0d0*E0/(1.0d0-DEXP(-E0/4.0d0))
        ENDIF
        IF(DABS(E0).LT.1.d-4) THEN
          BETA(32) = 120.0d0
        ELSE
          BETA(32) = 12.0d0*E0/(DEXP(E0/10.0d0)-1.0d0)
        ENDIF
        E0 = POT+34.0d0-SHIFT(33)
        IF(DABS(E0).LT.1.d-4) THEN
          ALPHA(33) = 25.0d0
        ELSE
          ALPHA(33) = 6.25d0*E0/(DEXP(E0/4.0d0)-1.0d0)
        ENDIF
        BETA(33) = 50.0d0/(1.0d0+DEXP(-E0/4.0d0))
      ENDIF

      IF(NAMODE.EQ.1) THEN
C *** For NAMODE=1  m & h equations based on Colatsky & Brown, 
C     Lee & Powell

        E0 = POT+41.0d0-SHIFT(10)
        IF(DABS(E0).LT.1.d-5) THEN
          ALPHA(10) = 2000.0d0
        ELSE
          ALPHA(10) = 200.0d0*E0/(1.0d0-DEXP(-0.1d0*E0))
        ENDIF
        BETA(10) = 8000.0d0*DEXP(-0.056d0*(POT+66.0d0-SHIFT(10)))
        ALPHA(9) = 20.0d0*DEXP(-0.125d0*(POT+75.0d0-SHIFT(9)))
        BETA(9)  = 2000.0d0/(320.0d0*DEXP(-0.1d0*(POT+75.0d0-
     '    SHIFT(9)))+1.0d0)

      ELSE IF(NAMODE.EQ.2) THEN
C ***   For NAMODE=2 use Calgary atrial rate equations

        E0 = POT-31.23d0-SHIFT(10)
        ALPHA(10) = 757.22d0*E0/(1.0d0-DEXP(-0.0617d0*E0))
        E0 = POT+52.853d0-SHIFT(10)
        BETA(10) = 2256.5d0*DEXP(-0.0418d0*E0)
        E0 = POT+85.45d0-SHIFT(9)
        ALPHA(9) = 51.1d0*DEXP(-0.1235d0*E0)
        E0 = POT+2.91d0-SHIFT(9)
        BETA(9) = 1270.7d0/(2.5d0+DEXP(-0.0764d0*E0))
      ENDIF

      RETURN
      END


      SUBROUTINE DFN_RATES_PLOT(IPARAM_1,IPARAM,NT_DATA,POTENTIALS,
     '  RESULTS,RATE)

C#### Subroutine: DFN_RATES_PLOT
C###  Description:
C###    Used to calculate a specified reaction rate coefficient pair

      IMPLICIT NONE
      !INCLUDE 'cell/deoxs00.cmn'
      !INCLUDE 'cell/oxs001.cmn'
      !INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs004.cmn'

      ! Parameter list
      INTEGER IPARAM_1(9),IPARAM(13),NT_DATA,RATE
      REAL*4 POTENTIALS(*),RESULTS(*)
      ! Local variables
      INTEGER i
      REAL*8 Y(33),RPARAM(9),FSS !,Em
      LOGICAL LPARAM(5)

C *** Inititialise the variables
      FSS = 0.0d0
      DO i=1,9
        RPARAM(i) = 0.0d0
      ENDDO
      DO i=1,5
        LPARAM(i) = .TRUE.
      ENDDO
      CALL DEOXS1(IPARAM_1)
      CALL DEOXS2(IPARAM,LPARAM,RPARAM)
      CALL OXSPREP(Y)

C *** For each potential value, calculate the rate coeffs
      DO i=1,NT_DATA
c        CALL DFN_RATES_SUB(ALPHA,BETA,SHIFT,SPEED,DBLE(POTENTIALS(i)),
c     '    FSS)
        CALL DFN_RATES_SUB(ALPHA,BETA,SHIFT,DBLE(POTENTIALS(i)),FSS)
        ! assign values to the results array
        RESULTS(i) = SNGL(ALPHA(RATE))
        RESULTS(i+NT_DATA) = SNGL(BETA(RATE))
      ENDDO

      RETURN
      END


      SUBROUTINE DIFRANCESCO_NOBLE(IPARAM_1,IPARAM_2,LPARAM_2,RPARAM,
     '  RPARAM_2,NT_DAT,Y_FULL)

C#### Subroutine: DIFRANCESCO_NOBLE
C###  Description:
C###    DiFrancesco-Noble (modified OXSOFT) model.

      IMPLICIT NONE
      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/oxs005.cmn'
      INCLUDE 'cell/deoxs00.cmn'
!     Passed variables
      INTEGER IPARAM_1(9),IPARAM_2(13),NT_DAT
      LOGICAL LPARAM_2(5)
      REAL*4 Y_FULL(*)
      REAL*8 RPARAM(21,9),RPARAM_2(9)
!     Common blocks
      INTEGER          NT_EQN,NT_CUR,NT_MECH,NO_FULL_MEM,NO_FULL_MECH,
     '                 NO_DAT,NUM_CUR
      COMMON /NUMBERS/ NT_EQN,NT_CUR,NT_MECH,NO_FULL_MEM,NO_FULL_MECH,
     '                 NO_DAT,NUM_CUR
!     Local variables
      INTEGER NO_EQN,NO_CUR,IFLAG
C     NT_MECH ,NO_DAT,NO_FULL,NT_EQN,NUM_CUR!NO_CON
      REAL*8 CUR(20),TCURRENT,TNEXT,Y_EQN(50)
!     SMAR009 23/12/98 CON(20),
      IF(DEBUG) write(*,'('' >>>call DIFRANCESCO_NOBLE'')')

      NT_EQN=33
      NUM_CUR=30
      NT_MECH=2
      
C *** Initialize
      CALL OXSINI(IPARAM_1,IPARAM_2,LPARAM_2,RPARAM,RPARAM_2,Y_EQN)
C?????DB.  Initial Y's are now passed in Y_FULL and not zeroing
C?????  variables from other models
      CALL OXSPREP(Y_EQN)
      DO NO_EQN=1,NT_EQN
        Y_EQN(NO_EQN)=DBLE(Y_FULL(NO_EQN))
      ENDDO
      NO_FULL_MEM=NT_EQN!+NUM_CUR+NT_MECH

C *** Write initial concentrations, currents and y's
c      CALL CONCUR(CON,CUR)
      CALL CONCUR(CUR)
      CALL LIOXSPARA(Y_EQN) !list parameters

      DO NO_CUR=1,NUM_CUR+NT_MECH
        NO_FULL_MEM=NO_FULL_MEM+1
        IF(NO_CUR.LE.20) THEN
          Y_FULL(NO_FULL_MEM)=SNGL(CUR(NO_CUR))
        ELSE
          Y_FULL(NO_FULL_MEM) = 0.0e0
        ENDIF
      ENDDO

C *** Calculate the solution.  Print and save after each TABT
      TCURRENT=TSTART
      IFLAG=0 !for 1st call to DESOL
      NO_DAT=1
      DO WHILE ((IFLAG.LE.1).AND.(NO_DAT.LT.NT_DAT))
        TNEXT=TCURRENT+TABT
        CALL DFN_CHANGE(TCURRENT,Y_EQN,IFLAG)
        CALL DESOL('DN',NT_EQN,TCURRENT,TNEXT,Y_EQN,IFLAG)
        IF(IFLAG.LE.1) THEN
C ***     Write concentrations, currents and y's
c          CALL CONCUR(CON,CUR)
          CALL CONCUR(CUR)
          DO NO_EQN=1,NT_EQN
            NO_FULL_MEM=NO_FULL_MEM+1
            Y_FULL(NO_FULL_MEM)=SNGL(Y_EQN(NO_EQN))
          ENDDO
C??? DPN. Saving currents for plotting
          DO NO_CUR=1,NUM_CUR+NT_MECH
            NO_FULL_MEM=NO_FULL_MEM+1
            IF(NO_CUR.LE.20) THEN
              Y_FULL(NO_FULL_MEM)=SNGL(CUR(NO_CUR))
            ELSE
              Y_FULL(NO_FULL_MEM) = 0.0e0
            ENDIF
          ENDDO
          NO_FULL_MEM=NO_FULL_MEM
          TCURRENT=TNEXT
          NO_DAT=NO_DAT+1
        ENDIF !iflag
      ENDDO !while iflag

      RETURN
      END


      SUBROUTINE DISTRIBUTION_MOMENT(IPARAM,RPARAM,RPARAM2,NT_DAT,
     '  Y_FULL,Y_EQN_DM,NT_EQN_DM,MEMBRANE_MODEL)

C#### Subroutine: DISTRIBUTION_MOMENT
C###  Description:
C###    Sets up the distribution moment model.
C###

      IMPLICIT NONE
!     Parameter List
      INTEGER IPARAM(3),NT_DAT,NT_EQN_DM,MEMBRANE_MODEL
      REAL*4 Y_FULL(*)
      REAL*8 RPARAM(50),RPARAM2(61),Y_EQN_DM(5)
!     Common blocks
      REAL*8      PI
      COMMON /PI/ PI
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      !Test protocols
      REAL*8        Pulse_interval
      COMMON /STIM/ Pulse_interval
      REAL*8        L
      COMMON /PROP/ L
      INTEGER          NT_EQN,NT_CUR,NT_MECH,NO_FULL_MEM,NO_FULL_MECH,
     '                 NO_DAT,NUM_CUR
      COMMON /NUMBERS/ NT_EQN,NT_CUR,NT_MECH,NO_FULL_MEM,NO_FULL_MECH,
     '                 NO_DAT,NUM_CUR
!     Local Variables

      PI=4.d0*ATAN(1.d0)
      NT_EQN_DM = 5
      NT_CUR    = 30
      NT_MECH   = 2
      Muscle_type = 1
      Integration = 1 !for distribution moment this implies no 
                      !numerical integration
      Icouple = 1 !weak coupling, needs to be passed through
      Pulse_interval = 0.1d3 !(ms??)

C     Initialise common block variables
      CALL FMINIT(IPARAM,RPARAM,RPARAM2,MEMBRANE_MODEL)

C     Place the initial tension and extension ratio values into 
C     results vector, and set the initial values for the solver.
C     DPN 3/12/97 - changing initial conditions
      Y_EQN_DM(1) = 5.0d-5 !initial total [Ca] - was 1.0d-8
      Y_EQN_DM(2) = 0.686d0 !initial stiffness - was 1.0d-6
      Y_EQN_DM(3) = 0.425d0 !initial force     - was 1.0e-6
      Y_EQN_DM(4) = 0.3d0 !initial energy      - was 2.0d-6
      Y_EQN_DM(5) = L      !initial extension ratio
      Y_FULL(NT_EQN+NT_CUR+1) = SNGL(Y_EQN_DM(5))
      Y_FULL(NT_EQN+NT_CUR+2) = SNGL(Y_EQN_DM(3)) !XXXX
      NO_FULL_MECH = NT_EQN+NT_CUR+NT_MECH
      RETURN
      END


      SUBROUTINE DN(T,Y,F)

C#### Subroutine: DN
C###  Description:
C###    Called by the Adams-Moulton ODE integrator.

      IMPLICIT NONE
      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs002.cmn'
      INCLUDE 'cell/oxs003.cmn'
!     Parameter List
      REAL*8 F(*),T,Y(*)

!      write(*,'('' >>>call dn'')')

C *** Call RATES except when instantaneous IV plots are being computed
      IF(.NOT.((PMODE.EQ.1).AND.PLOTT)) THEN
c        CALL DFN_RATES(T,Y,F)
        CALL DFN_RATES(Y,F)
      ENDIF

C *** In all cases compute currents and set voltage clamp if required
c      CALL DFN_CURRENTS(T,Y,F)
      CALL DFN_CURRENTS(Y,F)

C *** Compute changes in ion conc.s except when IV plots are being computed.
C *** Also compute when T=TSTART to obtain values for imNa and imCa for
C *** subroutines NAISS and CAISS.
      IF(.NOT.(PLOTT.AND.T.NE.TSTART)) THEN
c        CALL DFN_CONCENTRATIONS(T,Y,F)
        CALL DFN_CONCENTRATIONS(Y,F)
      ENDIF

      F1=F(1)

      RETURN
      END


      SUBROUTINE DM_INIT()

C#### Subroutine: DM_INIT
C###  Description:
C###    Initialises the distribution moment common blocks.

C ??? is this temporary? should these be passed from image map ???

      IMPLICIT NONE
!     Passed variables
!     Common blocks
      REAL*8 f1(9),g1(9),g2(9),f_dash0(9),f_dash1(9)
      COMMON /PARAMS1/ f1,g1,g2,f_dash0,f_dash1
      REAL*8 alpha(9),c_star(9),gamma(9),km(9),mu(9),rho(9),tau0(9)
      COMMON /PARAMS2/ alpha,c_star,gamma,km,mu,rho,tau0
      
      DATA f1/ 30., 26., 50. ,6*0/ !actomyosin bonding rate parameter
      DATA g1/  8., 15., 14. ,6*0/ !actomyosin unbonding rate parameter
      DATA g2/170.,300.,600. ,6*0/ !actomyosin unbonding rate parameter
      DATA f_dash0/20.,30.,50. ,6*0/
      DATA f_dash1/20.,30.,50. ,6*0/
      DATA alpha  / 1.0  ,1.0  ,1.0  ,6*0 / !fraction of Xbridges
      DATA c_star / 1.0  ,1.0  ,1.0  ,6*0 / !c* limiting [Ca]
      DATA gamma  / 0.020,0.015,0.015,6*0 / !sarcomere structural param 
      DATA km     / 0.006,0.006,0.006,6*0 / !Ca @ 1/2 max uptake by SR 
      DATA mu     / 0.01 ,0.10 ,0.20 ,6*0 / !Ca-troponin equil constant 
      DATA rho    / 2.0  ,1.6  ,1.3  ,6*0 / !Ca injection pulse magn.
      DATA tau0   / 0.25 ,0.10 ,0.125,6*0 / !1/max Ca uptake rate by SR

      RETURN
      END


      SUBROUTINE FADING_MEMORY(IPARAM,RPARAM,RPARAM2,NT_DAT,Y_FULL,
     '  NT_EQN2,t_beg,MEMBRANE_MODEL,LTRPNCa_from_JRW)

C#### Subroutine: FADING_MEMORY
C###  Description:
C###    Initialises the fading memory model with the specified 
C###    parameters. t_beg is returned with the value of the starting
C###    time for the fading memory model.

C***    Solves the fading memory model for a specified mechanical test.
C***    Y_FULL is returned containing the
C***    variable (L,T) values at each time step in the format:
C***       var1 t1, var2 t1, var3 t1,.....,var1 t2, var2 t2,... etc..
C***    

      IMPLICIT NONE
!     Parameter List
      INTEGER IPARAM(3),NT_DAT,NT_EQN2,MEMBRANE_MODEL
c      LOGICAL INITIALISED
      REAL*4 Y_FULL(*)
      REAL*8 RPARAM(50),RPARAM2(61),t_beg,LTRPNCa_from_JRW
!     Common blocks
      REAL*8      PI
      COMMON /PI/ PI
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      !To params
      REAL*8         Tref,beta0,beta1,beta2,gamma,Ca_scale
      COMMON /SS_To/ Tref,beta0,beta1,beta2,gamma,Ca_scale
      !Ca kinetics
      REAL*8      Ca_max,Ca_tau
      COMMON /Ca/ Ca_max,Ca_tau  
      !Tn kinetics
      REAL*8      Rho0,Rho1,Cb_max
      COMMON /Tn/ Rho0,Rho1,Cb_max
      !Tm kinetics 
      REAL*8      ALFA0,AA0,Tm_n_0,Tm_p50_0
      COMMON /Tm/ ALFA0,AA0,Tm_n_0,Tm_p50_0
      !Xb kinetics
      REAL*8      a,ALFA(3),AA(3)
      COMMON /XB/ a,ALFA,AA
      !Passive muscle
      REAL*8        a11,b11,k11
      COMMON /Tp_1/ a11,b11,k11 
      REAL*8        a22,b22,k22
      COMMON /Tp_2/ a22,b22,k22 
      REAL*8        a33,b33,k33
      COMMON /Tp_3/ a33,b33,k33 
      !Test protocols
      REAL*8        Pulse_interval
      COMMON /STIM/ Pulse_interval
      REAL*8        Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times(20),step_durations(20),
     '              step_magnitudes(20)
      INTEGER       current_step,number_of_steps
      COMMON /TEST/ Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times,step_durations,
     '              step_magnitudes,number_of_steps,current_step
      REAL*8      tt
      COMMON /TT/ tt
      REAL*8        L
      COMMON /PROP/ L
      REAL*8          MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      COMMON /MECH_T/ MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      REAL*8         Load
      COMMON /Load_/ Load
      REAL*8        mech_pCa,Ca_m,z,Cb,PHI(3),To_initial
      COMMON /VARS/ mech_pCa,Ca_m,z,Cb,PHI,To_initial
      INTEGER          count
      REAL*8           T
      COMMON /TEMPFIX/ T,count
      INTEGER          NT_EQN,NT_CUR,NT_MECH,NO_FULL_MEM,NO_FULL_MECH,
     '                 NO_DAT,NUM_CUR
      COMMON /NUMBERS/ NT_EQN,NT_CUR,NT_MECH,NO_FULL_MEM,NO_FULL_MECH,
     '                 NO_DAT,NUM_CUR
      REAL*8 Save_T,Save_To
      COMMON /COUPLING_TENSION/ Save_T,Save_to
!     Local Variables
      REAL*8 FN_ZSS,FN_TO

c dpn 07/05/98 unused int's - i,IFILE,N,NCHAR,Ntest,nTwitch,twitch,

c dpn 07/05/98 unused real*8's - A_Tm,C50_0,C50,Ca,Ca_new,Cb_new,current_time,dEXT,dGdL,dL,dPHI,dTodL,e11,e22,e33,EXT,E_arg,E_real,E_imag,E_mod,Fm,Fn,FM_INTEGRAND2,FN_CA,FN_TN,FN_TM,FN_Q,FN_TO_SS,FN_G,G,L1,L2,L3,lnw,LN_twoPI,Lprev,Q,tfinal,Tm_n,Tm_p50,To,Tp,TP1,TP2,TP3,t_twitch,w,w2,Y(5),z0,z_new

c dpn 07/05/98 unused complex*8's - CMPLX,E,SUM,

c dpn 07/05/98 unused logical's - Exit_program,

c dpn 07/05/98 unused character's - FILENAME*11,FILE_TYPE*2,MUSCLE(4)*23,TEST(15)*43,

C dpn 17/02/98      PARAMETER (TOL=1.d-6)     !tolerance for ode solver

      PI=4.d0*ATAN(1.d0)

      NT_CUR = 30
      NT_MECH = 2
      Muscle_type = 1
      Integration = 1 !for distribution moment this implies no 
                      !numerical integration

C     Initialise common block variables
      CALL FMINIT(IPARAM,RPARAM,RPARAM2,MEMBRANE_MODEL)

      IF(Test_type.EQ.8.OR.Test_type.EQ.9.OR.Test_type.EQ.-99) THEN
C       length step, multiple length steps, or non-coupled time
C       variable used
C dpn 17/02/98        N_tests=1
        !Integration=1
        mech_pCa = Tm_p50_0
        Ca_m = Ca_max 
        Cb   = 10**(6.d0-mech_pCa) !uM
        PHI(1) = 0.d0
        PHI(2) = 0.d0
        PHI(3) = 0.d0
C       Initial L now passed in as a parameter
C       L      = 1.d0
        z      = FN_ZSS(Cb,10**(6.d0-Tm_p50_0),Tm_n_0)
        To_initial = FN_TO(L,z) !initial isometric tension
        count=0
C dpn 17/02/98        mycount = 1
C       Place the initial tension and extension ratio values into 
C       results vector.
c        Y_FULL(NT_EQN+NT_CUR+1) = SNGL(Cb)
        Y_FULL(NT_EQN+NT_CUR+1) = SNGL(L)
        Y_FULL(NT_EQN+NT_CUR+2) = SNGL(To_initial)
        NO_FULL_MECH = NT_EQN+NT_CUR+NT_MECH
        t_beg = MECH_TSTART+MECH_DT
        Save_T = To_initial
        Save_To = To_initial
      ELSE IF(Test_type.EQ.5) THEN !isometric twitch
C dpn 17/02/98        N_tests=1
        !Integration=1
        Ca_m = Ca_max 
        Cb   = 0.d0 !initial bound Ca
        z    = 0.d0 !initial actin sites
C       Place the initial tension and extension ratio values into 
C       results vector.
        Y_FULL(NT_EQN+NT_CUR+1) = SNGL(L)
        Y_FULL(NT_EQN+NT_CUR+2) = SNGL(FN_TO(L,z))
        NO_FULL_MECH = NT_EQN+NT_CUR+NT_MECH
        count= 0
C dpn 17/02/98        mycount = 1
        t_beg = MECH_TSTART
        Save_T = FN_TO(L,z)
        Save_To = FN_TO(L,z)
      ELSE IF(Test_type.EQ.20) THEN 
        !coupled electro-mechanics isometric
C dpn 17/02/98        N_tests=1
        !Integration=1
c        Ca_m = Ca_max 
        IF (LTRPNCa_from_JRW.GT.0.0d0) THEN
          Cb = LTRPNCa_from_JRW
        ELSE
          Cb   = 0.d0 !initial bound Ca
        ENDIF
        z    = 0.d0 !initial actin sites
c        To_initial = FN_TO(L,z)
C       Place the initial tension and extension ratio values into 
C       results vector.
        Y_FULL(NT_EQN+NT_CUR+1) = SNGL(L)
        Y_FULL(NT_EQN+NT_CUR+2) = SNGL(FN_TO(L,z))
        NO_FULL_MECH = NT_EQN+NT_CUR+NT_MECH
        count= 0
C dpn 17/02/98        mycount = 1
        t_beg = MECH_TSTART
        Save_T = FN_TO(L,z)
        Save_To = FN_TO(L,z)
      ELSE IF(Test_type.EQ.21.OR.Test_type.EQ.99) THEN 
        !coupled electro-mechanics non-isometric or time variable used
C dpn 17/02/98        N_tests=1
        !Integration=1
        mech_pCa = Tm_p50_0
        IF (LTRPNCa_from_JRW.GT.0.0d0) THEN
          Cb = LTRPNCa_from_JRW
        ELSE
C          Cb   = 10**(6.d0-mech_pCa) !uM
          Cb = 0.0d0
        ENDIF
        PHI(1) = 0.d0
        PHI(2) = 0.d0
        PHI(3) = 0.d0
C       Initial L now passed in as a parameter
C      L      = 1.d0
C        z      = FN_ZSS(Cb,10**(6.d0-Tm_p50_0),Tm_n_0)
        z = 0.0d0
        To_initial = FN_TO(L,z) !initial isometric tension
        count=0
C dpn 17/02/98        mycount = 1
C       Place the initial tension and extension ratio values into 
C       results vector.
c        Y_FULL(NT_EQN+NT_CUR+1) = SNGL(Cb)
        Y_FULL(NT_EQN+NT_CUR+1) = SNGL(L)
        Y_FULL(NT_EQN+NT_CUR+2) = SNGL(To_initial)
        NO_FULL_MECH = NT_EQN+NT_CUR+NT_MECH
        t_beg = MECH_TSTART !+MECH_TABT
        Save_T = To_initial
        Save_To = To_initial
      ELSE IF(Test_type.EQ.14) THEN !isotonic shortening
        Cb=10.d0

        PHI(1) = 0.d0
        PHI(2) = 0.d0
        PHI(3) = 0.d0
        z      = FN_ZSS(Cb,10**(6.d0-Tm_p50_0),Tm_n_0)
        count=0
C dpn 17/02/98        mycount=1
C       Place the initial tension and extension ratio values into 
C       results vector.
        Y_FULL(NT_EQN+NT_CUR+1) = SNGL(L)
        Y_FULL(NT_EQN+NT_CUR+2) = 0.0e0
        NO_FULL_MECH = NT_EQN+NT_CUR+NT_MECH
        t_beg = MECH_TSTART+MECH_DT
        Save_T = 0.0d0
        Save_To = 0.0d0
      ENDIF
c      DO current_time=t_beg,TEND,TABT
c        IF(current_time.EQ.t_beg) THEN
c          CALL FM_SOLVE(current_time,current_time+TABT,count,T,tfinal)
C         assign new tension and extension ratio to results
c          NO_FULL = NO_FULL + NT_EQN + NT_CUR + NT_MECH
c          Y_FULL(NO_FULL-1) = SNGL(L) ! extension ratio
c          Y_FULL(NO_FULL) = SNGL(T) ! tension
c        ELSE
c          CALL FM_SOLVE(tfinal,current_time+TABT,count,T,tfinal)
C         assign new tension and extension ratio to results
c          NO_FULL = NO_FULL + NT_EQN + NT_CUR + NT_MECH
c          Y_FULL(NO_FULL-1) = SNGL(L) ! extension ratio
c          Y_FULL(NO_FULL) = SNGL(T) ! tension
c        ENDIF
c      ENDDO !current_time
C *** put in the final values
c      IF(Test_type.EQ.8.OR.Test_type.EQ.9) THEN
c        NO_FULL = NO_FULL + NT_EQN + NT_CUR + NT_MECH
c        Y_FULL(NO_FULL-1) = SNGL(L) ! extension ratio
c        Y_FULL(NO_FULL) = SNGL(T) ! tension
c      ELSE IF(Test_type.EQ.5) THEN
c        NO_FULL = NO_FULL + NT_EQN + NT_CUR + NT_MECH
c        Y_FULL(NO_FULL-1) = SNGL(L) ! extension ratio
c        Y_FULL(NO_FULL) = SNGL(FN_TO(L,z)) ! tension
c      ELSE IF(Test_type.EQ.14) THEN
c        NO_FULL = NO_FULL + NT_EQN + NT_CUR + NT_MECH
c        Y_FULL(NO_FULL-1) = SNGL(L) ! extension ratio
c        Y_FULL(NO_FULL) = SNGL(T) ! tension
c      ENDIF !Test_type
     
      RETURN
      END 


      SUBROUTINE FCN_DM(t,y,F)
      
C#### Subroutine: FCN_DM
C###  Description:
C###    Evaluates RHS vector of system of 1st order ODEs for 
C###    Distribution moment model.
C###      y(1) is c   (= [Ca]/m*)
C###      y(2) is Q0  (= muscle stiffness)
C###      y(3) is Q1  (= muscle force)
C###      y(4) is Q2  (= muscle energy)
C###      y(5) is Ext (= extension ratio)
C###
      
      IMPLICIT NONE
      INCLUDE 'cell/oxs005.cmn'
!     Passed variables
      REAL*8 t,y(*),F(*)
!     Common blocks
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8        Pulse_interval
      COMMON /STIM/ Pulse_interval
      REAL*8 alpha(9),c_star(9),gamma(9),km(9),mu(9),rho(9),tau0(9)
      COMMON /PARAMS2/ alpha,c_star,gamma,km,mu,rho,tau0
      REAL*8        Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times(20),step_durations(20),
     '              step_magnitudes(20)
      INTEGER       current_step,number_of_steps
      COMMON /TEST/ Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times,step_durations,
     '              step_magnitudes,number_of_steps,current_step
      REAL*8        r
      COMMON /ACTI/ r
!     Local variables
      INTEGER m,p
      REAL*8 u,chi,t_since_pulse
      REAL*8 c,Q0,Q1,Q2,cc!,Ext
      REAL*8 BETA_DM,PHI1,PHI2
      REAL*8 beta_0,beta_1,beta_2
      REAL*8 tau1,tau2
      DATA tau1/0.005/,tau2/0.001/
      
      m=Muscle_type

      c   = y(1)
      Q0  = y(2)
      Q1  = y(3)
      Q2  = y(4)
C dpn 17/02/98      Ext = y(5)
      r = c**2/(c**2+mu(m)*c+mu(m)**2)

      p=1
      chi=0.0d0
      DO WHILE(t.GT.DBLE(p-1)*Pulse_interval) !pth [Ca] pulse needed
        t_since_pulse=t-DBLE(p-1)*Pulse_interval
        chi=chi+(DEXP(-t_since_pulse/tau1)-DEXP(-t_since_pulse/tau2))
     '         /(tau1-tau2)
c        IF(IDOP.EQ.3) THEN
        IF(DEBUG) 
     '    write(*,'('' t='',D12.3,'' t_since_pulse '',I4,'' is '',
     '    D12.3,'' chi='',D12.3)') t,p,t_since_pulse,chi
c        ENDIF
        p=p+1
      ENDDO

      !Set scaled myofilament shortening velocity 
      IF(Test_type.EQ.5) THEN      !Isometric
        u = 0.0d0
      ELSE IF(Test_type.EQ.8) THEN !Step change in length
        IF(t.GE.Step_time.AND.t.LT.Step_time+Step_duration) THEN
          u =-Step_magnitude/Step_duration
        ELSE
          u = 0.0d0
        ENDIF
      ELSE IF(Test_type.EQ.9) THEN !Multiple step changes in length
        IF(t.GE.Step_time.AND.t.LT.Step_time+Step_duration) THEN
          u =-Step_magnitude/Step_duration
        ELSE
          u = 0.0d0
        ENDIF
      ELSE IF(Test_type.EQ.10) THEN !Length oscillation
        IF(t.GE.Osc_time) THEN
          u =-Osc_magnitude*Osc_freq*SIN(Osc_freq*(t-Osc_time))
        ELSE
          u = 0.0d0
        ENDIF
      ELSE 
        u = 0.0d0
      ENDIF
      beta_0 = BETA_DM(0)
      beta_1 = BETA_DM(1)
      beta_2 = BETA_DM(2)
      IF(Icouple.EQ.1) THEN      !Weak coupling
        cc  =c*c/(c*c-r*mu(m)+(2.d0+mu(m)/c)*r*(2.d0*c*(1.d0-r)-
     '    r*mu(m)))
        F(1)=cc*( rho(m)*(1.d0-c/c_star(m))*chi-(c/(c+km(m)))/tau0(m))
        F(2)=r*alpha(m)*beta_0 - r*PHI1(0,Q0,Q1,Q2)
     '                         -   PHI2(0,Q0,Q1,Q2)
        F(3)=r*alpha(m)*beta_1 - r*PHI1(1,Q0,Q1,Q2) 
     '                         -   PHI2(1,Q0,Q1,Q2) -    u*Q0
        F(4)=r*alpha(m)*beta_2 - r*PHI1(2,Q0,Q1,Q2) 
     '                         -   PHI2(2,Q0,Q1,Q2) - 2.0d0*u*Q1
        F(5)= -gamma(m)*u
      ELSE IF(Icouple.EQ.2) THEN !Strong coupling
        cc  =c*c/(c*c-r*mu(m)+(2.d0+mu(m)/c)*r*(2.d0*c*(1.d0-r)-
     '    r*mu(m)))
        F(1)=cc*( rho(m)*(1.d0-c/c_star(m))*chi-(c/(c+km(m)))/tau0(m))
        F(2)=r*alpha(m)*beta_0 - r*PHI1(0,Q0,Q1,Q2)
     '                         -   PHI2(0,Q0,Q1,Q2)
        F(3)=r*alpha(m)*beta_1 - r*PHI1(1,Q0,Q1,Q2) 
     '                         -   PHI2(1,Q0,Q1,Q2) -    u*Q0
        F(4)=r*alpha(m)*beta_2 - r*PHI1(2,Q0,Q1,Q2) 
     '                         -   PHI2(2,Q0,Q1,Q2) - 2.0d0*u*Q1
        F(5)= -gamma(m)*u
      ENDIF
        
      RETURN
      END


      SUBROUTINE FCN_JRWP(j,K1_inf_const,icell_type,Vinput,state,
     $  Fstate,mem_current)
      
      RETURN
      END


      SUBROUTINE FMINIT(IPARAM,RPARAM,RPARAM2,MEMBRANE_MODEL)

C#### Subroutine: FMINIT
C###  Description:
C###    Takes the array of parameter values passed into FADING_MEMORY,
C###    and assigns them to the correct variables in the common blocks

      IMPLICIT NONE
!     Parameter List
      INTEGER IPARAM(3),MEMBRANE_MODEL
      REAL*8 RPARAM(52),RPARAM2(61)
!     Common Blocks
!Passive tension-length parameters
      REAL*8        a11,b11,k11
      COMMON /Tp_1/ a11,b11,k11 
      REAL*8        a22,b22,k22
      COMMON /Tp_2/ a22,b22,k22 
      REAL*8        a33,b33,k33
      COMMON /Tp_3/ a33,b33,k33 
!SS tension-length-pCa parameters
      REAL*8         Tref,beta0,beta1,beta2,gamma,Ca_scale
      COMMON /SS_To/ Tref,beta0,beta1,beta2,gamma,Ca_scale
!Calcium twitch parameters
      REAL*8      Ca_max,Ca_tau
      COMMON /Ca/ Ca_max,Ca_tau  
!Intracellular Ca kinetics
!Troponin kinetics
      REAL*8      Rho0,Rho1,Cb_max
      COMMON /Tn/ Rho0,Rho1,Cb_max
!Tropomyosin kinetics 
      REAL*8      ALFA0,AA0,Tm_n_0,Tm_p50_0
      COMMON /Tm/ ALFA0,AA0,Tm_n_0,Tm_p50_0
!Fading memory parameters
      REAL*8 a,ALFA(3),AA(3)
      COMMON /XB/ a,ALFA,AA
      REAL*8        L
      COMMON /PROP/ L
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8        Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times(20),step_durations(20),
     '              step_magnitudes(20)
      INTEGER       current_step,number_of_steps
      COMMON /TEST/ Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times,step_durations,
     '              step_magnitudes,number_of_steps,current_step
      REAL*8          MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      COMMON /MECH_T/ MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      REAL*8         Load
      COMMON /Load_/ Load
!     Local variables
      INTEGER i

C Assign values from RPARAM to the common block variables
C converting times from ms to seconds
      L              = RPARAM(1+1)
      Rho0            = RPARAM(2+1) * 1.0d+3
      Rho1           = RPARAM(3+1) * 1.0d+3
      Cb_max         = RPARAM(4+1)
      ALFA0          = RPARAM(5+1) * 1.0d+3
      AA0            = RPARAM(6+1)
      Tm_n_0         = RPARAM(7+1)
      Tm_p50_0       = RPARAM(8+1)
      a11            = RPARAM(9+1)
      b11            = RPARAM(10+1)
      k11            = RPARAM(11+1)
      a22            = RPARAM(12+1)
      b22            = RPARAM(13+1)
      k22            = RPARAM(14+1)
      a33            = RPARAM(15+1)
      b33            = RPARAM(16+1)
      k33            = RPARAM(17+1)
      a              = RPARAM(18+1)
      ALFA(1)        = RPARAM(19+1) * 1.0d+3
      ALFA(2)        = RPARAM(20+1) * 1.0d+3
      ALFA(3)        = RPARAM(21+1) * 1.0d+3
      AA(1)          = RPARAM(22+1)
      AA(2)          = RPARAM(23+1)
      AA(3)          = RPARAM(24+1)
      Tref           = RPARAM(25+1)
      beta0          = RPARAM(26+1)
      beta1          = RPARAM(27+1)
      beta2          = RPARAM(28+1)
      Ca_max         = RPARAM(29+1)
      Ca_tau         = RPARAM(30+1) * 1.0d-3
      Step_time      = RPARAM(31+1) * 1.0d-3
      Step_magnitude = RPARAM(32+1)
      Step_duration  = RPARAM(33+1) * 1.0d-3
      MECH_TIMESCALE = RPARAM(34+1)
      MECH_TSTART    = RPARAM(35+1) * 1.0d-3
      MECH_TEND      = RPARAM(36+1) * 1.0d-3
      MECH_DT        = RPARAM(37+1) * 1.0d-3
      MECH_TABT      = RPARAM(38+1) * 1.0d-3
      MECH_TPS       = RPARAM(39+1) * 1.0d-3
      TP_mem         = RPARAM(40+1) * 1.0d-3
      gamma           = RPARAM(42)
      Ca_scale       = RPARAM(43)
C *** Get the test type
      IF(IPARAM(1).EQ.1) THEN !isometric twitch
        Test_type = 5
      ELSE IF(IPARAM(1).EQ.2) THEN !step change in length
        Test_type = 8
      ELSE IF(IPARAM(1).EQ.3) THEN !multiple length steps
        Test_type = 9
      ELSE IF(IPARAM(1).EQ.4) THEN !sinusoidal test
        Test_type = 10
      ELSE IF(IPARAM(1).EQ.5) THEN !isotonic shortening
        Test_type = 14
      ELSE IF(IPARAM(1).EQ.6) THEN !coupled electro-mechanical 
                                   !isometric test
        Test_type = 20
      ELSE IF(IPARAM(1).EQ.7) THEN !coupled electro-mechanical 
                                   !non-isometric test
        Test_type = 21
      ELSE IF(IPARAM(1).EQ.99) THEN !time variable used
        IF (MEMBRANE_MODEL.EQ.0.OR.MEMBRANE_MODEL.EQ.2) THEN
          Test_type = -99 !fading memory only i.e. non-coupled
        ELSE
          Test_type = 99 !coupled model
        ENDIF
      ENDIF
C if multiple steps, get the appropriate parameters
      IF(Test_type.EQ.9) THEN
        number_of_steps      = INT(RPARAM2(61))
        current_step         = 1
        DO i=1,number_of_steps
          step_times(i)      = RPARAM2(i) * 1.0d-3
          step_magnitudes(i) = RPARAM2(i+20)
          step_durations(i)  = RPARAM2(i+40) * 1.0d-3
        ENDDO
      ELSE IF(Test_type.EQ.14) THEN
        Load      = RPARAM2(1)
        Step_time = 0.05d0
      ENDIF
      RETURN
      END 


      REAL*8 FUNCTION FM_INTEGRAND1(FMterm,tau,EXTENSION_RATIO_VALUES)

C#### Function: FM_INTEGRAND1
C###  Type: REAL*8
C###  Description:
C###    Evaluate integrand for one component of fading memory 
C###    hereditary integral. FMterm is 1, 2 or 3 for each rate 
C###    constant term.

      IMPLICIT NONE
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      INTEGER FMterm
      REAL*4  EXTENSION_RATIO_VALUES(*)
      REAL*8  tau
!     Common Blocks
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8      a,ALFA(3),AA(3)
      COMMON /XB/ a,ALFA,AA
      REAL*8        Pulse_interval
      COMMON /STIM/ Pulse_interval
      REAL*8        Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times(20),step_durations(20),
     '              step_magnitudes(20)
      INTEGER       current_step,number_of_steps
      COMMON /TEST/ Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times,step_durations,
     '              step_magnitudes,number_of_steps,current_step
      REAL*8      tt
      COMMON /TT/ tt
      REAL*8          MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      COMMON /MECH_T/ MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      INTEGER                     current_time_step,previous_time_step
      COMMON /TIME_VARIABLE_INFO/ current_time_step,previous_time_step
!     Local Variables
      REAL*8 u

      !Set scaled myofilament shortening velocity 
      IF(Test_type.EQ.7) THEN       !Isometric twitch
        u = 0.d0   

C      ELSE IF(Test_type.EQ.8.OR.Test_type.EQ.9) THEN !Step change in 
                                                     !length
C      ELSE IF(Test_type.EQ.8) THEN !step change in length
      ELSE IF(Test_type.EQ.8.OR.Test_type.EQ.21) THEN !step change 
                                                      !in length
!        write(*,'('' times='',2E12.3,'' tau='',E12.3)')
!     '    Step_time,Step_time+Step_duration,tau
        IF(tt-tau.GE.Step_time.AND.tt-tau.LT.Step_time+
     '    Step_duration) THEN
          u = Step_magnitude/Step_duration
c          write(*,*) "tau = ",tau
c          write(*,*) "tt = ",tt
c          write(*,*)
        ELSE
          u = 0.d0   
c          write(*,*) "u = 0 tau = ",tau
c          write(*,*) "u = 0 tt = ",tt
c          write(*,*)
        ENDIF
      ELSE IF(Test_type.EQ.9) THEN !multiple length steps
        IF(tt-tau.GE.step_times(current_step).AND.
     '    tt-tau.LT.step_times(current_step)+
     '    step_durations(current_step)) THEN
          u = step_magnitudes(current_step)/step_durations(current_step)
          current_step = current_step + 1
        ELSE
          u = 0.d0   
        ENDIF
C      ELSE IF(Test_type.EQ.8.      !Step change in length
C     '     OR.Test_type.EQ.9) THEN  
!        write(*,'('' times='',2E12.3,'' tt-tau='',E12.3)')
!     '    Step_time,Step_time+Step_duration,tt-tau
C        IF(tt-tau.GE.Step_time.AND
C     '    .tt-tau.LT.Step_time+Step_duration) THEN
C          u = Step_magnitude/Step_duration
C        ELSE
C          u = 0.d0   
C        ENDIF

      ELSE IF(Test_type.EQ.10) THEN !Stiffness
        IF(tt-tau.GE.Osc_time) THEN
          u =Osc_magnitude*Osc_freq*DSIN(Osc_freq*(tt-tau-Osc_time))
        ELSE
          u = 0.d0   
        ENDIF

      ELSE IF(Test_type.EQ.14) THEN !Isotonic shortening
        u = Step_magnitude/Step_duration

      ELSE IF(ABS(Test_type).EQ.99) THEN !time variable used
        IF(previous_time_step.EQ.0) THEN
          u = 0.0d0
        ELSE
          u = DBLE(EXTENSION_RATIO_VALUES(current_time_step) -
     '      EXTENSION_RATIO_VALUES(previous_time_step)) / MECH_TABT
        ENDIF
      ELSE 
        u = 0.d0   
      ENDIF

      FM_INTEGRAND1=DEXP(-ALFA(FMterm)*tau)*u

c      IF(IDOP.EQ.3) THEN
        IF(DEBUG) WRITE(*,'('' tau= '',E10.3,'' u='',E12.3,'
     '    //''' FM_INTEGRAND1='',E12.3)') tau,u,FM_INTEGRAND1
c      ENDIF

      RETURN
      END


      REAL*8 FUNCTION FM_INTEGRAND2(tau,EXTENSION_RATIO_VALUES,STEP)

C#### Function: FM_INTEGRAND2
C###  Type: REAL*8
C###  Description:
C###    Evaluate integrand for integral of velocity.

      IMPLICIT NONE
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      REAL*4 EXTENSION_RATIO_VALUES(*)
      REAL*8 tau,STEP
!     Common Blocks
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8        Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times(20),step_durations(20),
     '              step_magnitudes(20)
      INTEGER       current_step,number_of_steps
      COMMON /TEST/ Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times,step_durations,
     '              step_magnitudes,number_of_steps,current_step
      REAL*8          MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      COMMON /MECH_T/ MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      INTEGER                     current_time_step,previous_time_step
      COMMON /TIME_VARIABLE_INFO/ current_time_step,previous_time_step

!     Local Variables
      REAL*8 u

      ! Set scaled myofilament shortening velocity 
      IF(Test_type.EQ.7) THEN      !Isometric
        u = 0.d0   

C      ELSE IF(Test_type.EQ.8.OR.Test_type.EQ.9) THEN !Step change in 
                                                     !length
C ***  DPN 12/01/98 - adding non-isometric coupled model
C      ELSE IF(Test_type.EQ.8) THEN !step change in length
      ELSE IF(Test_type.EQ.8.OR.Test_type.EQ.21) THEN !step change
                                                      !in length
!        write(*,'('' times='',2E12.3,'' tau='',E12.3)')
!     '    Step_time,Step_time+Step_duration,tau
        IF(tau.GE.Step_time.AND.tau.LT.Step_time+Step_duration) THEN
          u = Step_magnitude/Step_duration
        ELSE
          u = 0.d0   
        ENDIF
      ELSE IF(Test_type.EQ.9) THEN !multiple length steps
        IF(tau.GE.step_times(current_step).AND.
     '    tau.LT.step_times(current_step)+
     '    step_durations(current_step)) THEN
          u = step_magnitudes(current_step)/step_durations(current_step)
          current_step = current_step + 1
        ELSE
          u = 0.d0   
        ENDIF
      ELSE IF(Test_type.EQ.10) THEN !Stiffness
        IF(tau.GE.Osc_time) THEN
          u = Osc_magnitude*Osc_freq*DSIN(Osc_freq*(tau-Osc_time))
        ELSE
          u = 0.d0   
        ENDIF

      ELSE IF(Test_type.EQ.14) THEN !Isotonic shortening
        u = Step_magnitude/Step_duration

      ELSE IF(ABS(Test_type).EQ.99) THEN !coupled, time variable used
        IF(previous_time_step.EQ.0) THEN
          u = 0.0d0
        ELSE
c          u = DBLE(EXTENSION_RATIO_VALUES(current_time_step) -
c     '      EXTENSION_RATIO_VALUES(previous_time_step)) / MECH_TABT
          u = DBLE(EXTENSION_RATIO_VALUES(current_time_step) -
     '      EXTENSION_RATIO_VALUES(previous_time_step)) / STEP
        ENDIF
      ELSE 
        u = 0.d0   
      ENDIF

      FM_INTEGRAND2 = u

c      IF(IDOP.EQ.3) THEN
        IF(DEBUG) WRITE(*,'('' tau= '',E10.3,'' u='',E12.3,'
     '    //''' FM_INTEGRAND2='',E12.3)') tau,u,FM_INTEGRAND2
c      ENDIF

      RETURN
      END


      SUBROUTINE FM_SOLVE(TBEG,TNEXT,count,T,TFINAL,
     '  EXTENSION_RATIO_VALUES)

C#### Subroutine: FM_SOLVE
C###  Description:
C###    Solves the Fading Memory model for a given test, between
C###    TBEG and TNEXT

      IMPLICIT NONE
      INCLUDE 'cell/oxs005.cmn'
!     Parameter list
      INTEGER count
      REAL*4 EXTENSION_RATIO_VALUES(*)
      REAL*8 TBEG,TNEXT,T,TFINAL
!     Common Blocks
      REAL*8      PI
      COMMON /PI/ PI
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      !To params
      REAL*8         Tref,beta0,beta1,beta2,gamma,Ca_scale
      COMMON /SS_To/ Tref,beta0,beta1,beta2,gamma,Ca_scale
      !Ca kinetics
      REAL*8      Ca_max,Ca_tau
      COMMON /Ca/ Ca_max,Ca_tau  
      !Tn kinetics
      REAL*8      Rho0,Rho1,Cb_max
      COMMON /Tn/ Rho0,Rho1,Cb_max
      !Tm kinetics 
      REAL*8      ALFA0,AA0,Tm_n_0,Tm_p50_0
      COMMON /Tm/ ALFA0,AA0,Tm_n_0,Tm_p50_0
      !Xb kinetics
      REAL*8      a,ALFA(3),AA(3)
      COMMON /XB/ a,ALFA,AA
      !Passive muscle
      REAL*8        a11,b11,k11
      COMMON /Tp_1/ a11,b11,k11 
      REAL*8        a22,b22,k22
      COMMON /Tp_2/ a22,b22,k22 
      REAL*8        a33,b33,k33
      COMMON /Tp_3/ a33,b33,k33 
      REAL*8        L
      COMMON /PROP/ L
      !Test protocols
      REAL*8        Pulse_interval
      COMMON /STIM/ Pulse_interval
      REAL*8        Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times(20),step_durations(20),
     '              step_magnitudes(20)
      INTEGER       current_step,number_of_steps
      COMMON /TEST/ Step_time,Step_magnitude,Step_duration,
     '              Max_step_magnitude,
     '              Osc_time,Osc_magnitude,Osc_period,Osc_freq,
     '              step_times,step_durations,
     '              step_magnitudes,number_of_steps,current_step
      REAL*8      tt
      COMMON /TT/ tt
      REAL*8          MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      COMMON /MECH_T/ MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      REAL*8         Load
      COMMON /Load_/ Load
      REAL*8        mech_pCa,Ca_m,z,Cb,PHI(3),To_initial
      COMMON /VARS/ mech_pCa,Ca_m,z,Cb,PHI,To_initial
      REAL*8             Ca_from_membrane,Ca_new_from_membrane,
     '  LTRPNCa_from_JRW
      COMMON /STORED_CA/ Ca_from_membrane,Ca_new_from_membrane,
     '  LTRPNCa_from_JRW
      REAL*8 Save_T,Save_To
      COMMON /COUPLING_TENSION/ Save_T,Save_to
      INTEGER                     current_time_step,previous_time_step
      COMMON /TIME_VARIABLE_INFO/ current_time_step,previous_time_step
      REAL*8 my_To,my_T,my_Cb,my_z,my_time,my_Q
      COMMON /MY_STUFF/ my_To,my_T,my_Cb,my_z,my_time,my_Q
!     Local Variables
      REAL*8 dL,FM_INTEGRAND2,Fm,FN_TM,To,FN_TO,Q,FN_Q,Ca,FN_CA,Ca_new,
     '  Fn,FN_TN,Cb_new,z_new,Lprev,z0,G,FN_G,dGdL,dPHI
c      INTEGER twitch,nTwitch,NT_CUR,NT_MECH,NO_FULL
c      REAL*8 lnw,w,w2,To,dTodL,C50_0,A_Tm
c      REAL*8 dL,z_new,Fm,Fn !,L
c      REAL*8 Tp,TP1,TP2,TP3
c      REAL*8 t_twitch,Ca,Ca_new,Cb_new !,dt
c      REAL*8 C50,Tm_n,Tm_p50
c      REAL*8 TOL,Y(5) !,Tend,Tbegin
c      REAL*8 L1,L2,L3,e11,e22,e33
c      REAL*8 EXT,dEXT,Q,t_beg
c      REAL*8 FN_CA,FN_TN,FN_TM,FN_ZSS,FN_Q,FN_TO,FN_TO_SS,
c     '  FM_INTEGRAND2
c      REAL*8 z0,FN_G,G,dGdL,dPHI,Lprev
c      LOGICAL Exit_program
c      COMPLEX*8 CMPLX,E,SUM
c      REAL*8 E_real,E_imag,E_mod,E_arg,LN_twoPI
c      CHARACTER FILENAME*11,FILE_TYPE*2,MUSCLE(4)*23,TEST(15)*43

!MLB initialising CA - used before set
c      Ca=0.0d0

C      write(*,'('' >>>call FM_SOLVE - i.e. Mechanics solving at '''//
C     '  ',D11.5)') TBEG

C *** if not solving a coupled system, loop through the tabulation
C *** interval
C *** else, solve only once, i.e., the time step is the tabulation
C *** interval
      IF (Test_type.LT.20) THEN

      DO tt = TBEG,TNEXT,MECH_DT
        count=count+1
        IF(count.EQ.5) count=0
        IF (Test_type.EQ.8.OR.Test_type.EQ.9
     '    .OR.Test_type.EQ.-99) THEN !length steps, or 
                                     !non-coupled time variable
C ***     DPN 30 April 98 - time variable info
          previous_time_step = current_time_step
          current_time_step = current_time_step + 1

C ??? DPN 12/01/1998 - No troponin kinetics due to maximally 
C ???                  activated cell ???

C        dL = 0.5d0*(FM_INTEGRAND2(tt-dt)+FM_INTEGRAND2(tt))*dt
          Ca     = 0.01d0 + FN_CA(tt   ,Ca_m)
          Ca_new = 0.01d0 + FN_CA(tt+MECH_DT,Ca_m)
          mech_pCa=6.d0-DLOG10(Ca) !Ca is in uM
          Fn = FN_TN(Ca,Cb,T,T) !troponin kinetics
          Fm = FN_TM(Cb,z,L) !tropomyosin kinetics
          Cb_new = Cb + MECH_DT*Fn !prediction step for Cb
          z_new  =  z + MECH_DT*Fm !prediction step for z

          dL = 0.5d0*(FM_INTEGRAND2(tt-MECH_DT,EXTENSION_RATIO_VALUES,
     '      MECH_DT)+
     '      FM_INTEGRAND2(tt,EXTENSION_RATIO_VALUES,MECH_DT))*
     '      MECH_DT
          L  = L+dL !new length

          Cb = Cb + 0.5d0*MECH_DT*(Fn+FN_TN(Ca_new,Cb_new,T,T)) !new Cb
          z  =  z + 0.5d0*MECH_DT*(Fm+FN_TM(Cb_new,z_new,L)) !new z

ccc          Fm = FN_TM(Cb,z,L) !tropomyosin kinetics
C        z  = z + dt*Fm !prediction step for z
ccc          z  = z + MECH_DT*Fm !prediction step for z
          To = FN_TO(L,z) !isometric tension
C        Q  = FN_Q(PHI,dt) !Xbridge kinetics
          Q  = FN_Q(PHI,MECH_DT,EXTENSION_RATIO_VALUES)!Xbridge kinetics
          T  = To*(1.d0+a*Q)/(1.d0-Q) !tension
                  
c          write(*,'(f5.3,e12.3,e12.3,e12.3,e12.3,e12.3,e12.3)') 
c     '      tt,Ca,Cb,L,To,T,T/To

          IF(count.EQ.1) THEN !output
            IF(tt.GE.Step_time-0.01d0.AND.tt.LE.0.1d0) THEN
              IF(DEBUG) WRITE(*,'(  '' tt='',F5.3,''(s)'','
     '          //'  '' Cb='',F6.4,''(uM)'','
     '          //'  '' dL='',F6.4,'
     '          //'   '' L='',F6.4,'
     '          //'   '' z='',F5.2,'
     '          //'  '' To='',F6.2,'
     '          //'   '' Q='',F6.2,'
     '          //'   '' T='',F6.2,'
     '          //''' T/To='',F6.2)') 
     '          tt,Cb,dL,L,z,To,Q,T,T/To_initial
            ENDIF
C          IF(Test_type.EQ.8) THEN
C            WRITE(3,'(1X,2E15.5)') tt,T
C          ELSE IF(Test_type.EQ.9) THEN
C            WRITE(3,'(1X,2E15.5)') tt,T/To_initial
C          ENDIF !Test_type
          ENDIF !count
          my_time = tbeg
          my_Cb = Cb
          my_z = z
          my_To = To
          my_Q = Q
          my_T = Ca
        ELSE IF(Test_type.EQ.5) THEN
          Ca     = 0.01d0 + FN_CA(tt   ,Ca_m)
          Ca_new = 0.01d0 + FN_CA(tt+MECH_DT,Ca_m)
          mech_pCa=6.d0-DLOG10(Ca) !Ca is in uM

          Fn = FN_TN(Ca,Cb,T,T) !troponin kinetics
          Fm = FN_TM(Cb,z,L) !tropomyosin kinetics
          Cb_new = Cb + MECH_DT*Fn !prediction step for Cb
          z_new  =  z + MECH_DT*Fm !prediction step for z
          Cb = Cb + 0.5d0*MECH_DT*(Fn+FN_TN(Ca_new,Cb_new,T,T)) !new Cb
          z  =  z + 0.5d0*MECH_DT*(Fm+FN_TM(Cb_new,z_new,L)) !new z

          T = FN_TO(L,z)

c          write(*,'(f5.3,e12.3,e12.3,e12.3,e12.3)') 
c     '      tt,Ca,Cb,T,z

          IF(count.EQ.1) THEN !output
            IF(DEBUG) WRITE(*,'('' t='',F5.3,'' Ca='',E12.3,''(uM)'''
     '        //''' pCa='',E12.3,'
     '        //' '' Cb='',E12.3,''(uM)'''
     '        //'  '' z='',E12.4)') 
     '        tt,Ca,mech_pCa,Cb,z
          ENDIF !count
        ELSE IF(Test_type.EQ.14) THEN 
          Lprev=L
          z0= z
          G = 1.d6 !just large initial value
          DO WHILE(DABS(G).GT.1.d-6) !Newton iterations
            G    =  FN_G(tt,MECH_DT,L      ,Lprev,Cb,z0,PHI,T,To,Load)
            dGdL = (FN_G(tt,MECH_DT,L+1.d-4,Lprev,Cb,z0,PHI,T,To,Load)
     '        -G)/1.d-4
            L    =  L - G/dGdL !new length
c            IF(IDOP.EQ.3) THEN
              IF(DEBUG) write(*,'('' L='',E12.3,'' G='',E12.3)') L,G
c            ENDIF
          ENDDO

          z=z0
          dPHI=(L-Lprev)/(ALFA(1)*MECH_DT)
     '      *DEXP(ALFA(1)*tt)*(1.d0-DEXP(-ALFA(1)*MECH_DT))
          PHI(1)=PHI(1)+dPHI

c          IF(count.EQ.1) THEN !output
c            WRITE(*,'(  '' tt='',F6.4,''(s)'','
c     '        //'  '' Cb='',F6.3,''(uM)'','
c     '        //'   '' L='',F6.4,'
c     '        //'   '' z='',F5.3,'
c     '        //'  '' To='',F7.3,'
c     '        //'   '' T='',F7.3,'
c     '        //'   '' V='',E11.3)')
c     '        tt,Cb,L,z,To,T,(L-Lprev)/MECH_DT
c            WRITE(3,'(1X,2E15.5)') tt,T
c          ENDIF !count
        ENDIF !Test_type
      ENDDO !tt
      TFINAL = tt

      ELSE !solving coupled system, solve once.

        count=count+1
        IF(count.EQ.5) count=0
C ***   use the [Ca]i from the membrane model
C ***   DPN 02/02/98 - add Ca_scale
        Ca     = Ca_from_membrane * Ca_scale
        Ca_new = Ca_new_from_membrane * Ca_scale
        mech_pCa=6.d0-DLOG10(Ca) !Ca is in uM
        tt = TBEG
        IF (Test_type.EQ.20) THEN !isometric test

C ??? DPN 12/01/98 - no assumption of maximally activated cell ???

C *** DPN 12/06/98 - use LTRPNCa if using JRW membrane model
          IF (LTRPNCa_from_JRW.LT.0.0d0) THEN
            Fn = FN_TN(Ca,Cb,T,T) !troponin kinetics
            Fm = FN_TM(Cb,z,L) !tropomyosin kinetics
            Cb_new = Cb + MECH_TABT*Fn !prediction step for Cb
            z_new  =  z + MECH_TABT*Fm !prediction step for z
            Cb = Cb + 0.5d0*MECH_TABT*(Fn+FN_TN(Ca_new,Cb_new,T,T)) !newCb
            z  =  z + 0.5d0*MECH_TABT*(Fm+FN_TM(Cb_new,z_new,L)) !new z
          ELSE
            Fm = FN_TM(Cb,z,L) !tropomyosin kinetics
            Cb = LTRPNCa_from_JRW
            z_new  =  z + MECH_TABT*Fm
            z  =  z + 0.5d0*MECH_TABT*(Fm+FN_TM(Cb,z_new,L)) !new z
          ENDIF !
          T = FN_TO(L,z)
          Save_T = T
          Save_To = T
        ELSE IF (Test_type.EQ.21) THEN !non-isometric test
C *** DPN 12/01/98 - adding non-isometric test (step change in length)
          Fn = FN_TN(Ca,Cb,T,To) !troponin kinetics
          dL = 0.5d0*(
     '      FM_INTEGRAND2(TBEG-MECH_TABT,EXTENSION_RATIO_VALUES,
     '      MECH_TABT)+
     '      FM_INTEGRAND2(TBEG,EXTENSION_RATIO_VALUES,MECH_TABT))
     '      *MECH_TABT
          Fm = FN_TM(Cb,z,L) !tropomyosin kinetics
          L  = L+dL !new length
C *** DPN 12/06/98 - use LTRPNCa if using JRW membrane model
          IF (LTRPNCa_from_JRW.LT.0.0d0) THEN
            Cb_new = Cb + MECH_TABT*Fn !prediction step for Cb
            z_new  =  z + MECH_TABT*Fm !prediction step for z
            Cb = Cb + 0.5d0*MECH_TABT*(Fn+FN_TN(Ca_new,Cb_new,T,To))!new Cb
            z  =  z + 0.5d0*MECH_TABT*(Fm+FN_TM(Cb_new,z_new,L)) !new z
          ELSE
            z_new  =  z + MECH_TABT*Fm !prediction step for z
            Cb = LTRPNCa_from_JRW
            z  =  z + 0.5d0*MECH_TABT*(Fm+FN_TM(Cb,z_new,L)) !new z
          ENDIF
          To = FN_TO(L,z) !isometric tension
          Q  = FN_Q(PHI,MECH_TABT,EXTENSION_RATIO_VALUES)!Xbridge kinetics
          T  = To*(1.d0+a*Q)/(1.d0-Q) !tension
          Save_T = T
          Save_To = To
        ELSE IF (Test_type.EQ.99) THEN !coupled, time variable used
C ***     DPN 30 April 98 - time variable info
          previous_time_step = current_time_step
          current_time_step = current_time_step + 1
          Fn = FN_TN(Ca,Cb,Save_T,Save_To) !troponin kinetics
C ***     DPN 30 April 98 - use the values from the time variable
          dL = 0.5d0*(FM_INTEGRAND2(TBEG-MECH_TABT,
     '      EXTENSION_RATIO_VALUES,MECH_TABT)+
     '      FM_INTEGRAND2(TBEG,EXTENSION_RATIO_VALUES,MECH_TABT))
     '      *MECH_TABT
          Fm = FN_TM(Cb,z,L) !tropomyosin kinetics
          L  = L+dL !new length
C *** DPN 12/06/98 - use LTRPNCa if using JRW membrane model
          
          IF (LTRPNCa_from_JRW.LT.0.0d0) THEN
            Cb_new = Cb + MECH_TABT*Fn !prediction step for Cb
            z_new  =  z + MECH_TABT*Fm !prediction step for z
            Cb = Cb + 0.5d0*MECH_TABT*(Fn+FN_TN(Ca_new,Cb_new,T,To))!new Cb
            z  =  z + 0.5d0*MECH_TABT*(Fm+FN_TM(Cb_new,z_new,L)) !new z
          ELSE
            z_new  =  z + MECH_TABT*Fm !prediction step for z
            Cb = LTRPNCa_from_JRW
            z  =  z + 0.5d0*MECH_TABT*(Fm+FN_TM(Cb,z_new,L)) !new z
          ENDIF
          To = FN_TO(L,z) !isometric tension
          Q  = FN_Q(PHI,MECH_TABT,EXTENSION_RATIO_VALUES)!Xbridge kinetics
          T  = To*(1.d0+a*Q)/(1.d0-Q) !tension
          Save_T = T
          Save_To = To
C ***     DPN 29/09/98 - might be useful (plot in UnEMAP)??
          my_time = tbeg
          my_Cb = Cb
          my_z = z
          my_To = To
          my_Q = Q
          my_T = T
        ENDIF
        IF(count.EQ.1) THEN !output
          IF(DEBUG) WRITE(*,'('' t='',F5.3,'' Ca='',E12.3,''(uM)'''
     '      //''' pCa='',E12.3,'
     '      //' '' Cb='',E12.3,''(uM)'''
     '      //'  '' z='',E12.4)') 
     '      TBEG,Ca,mech_pCa,Cb,z
        ENDIF !count
        TFINAL = TBEG + MECH_TABT
      ENDIF !coupled/non-coupled system
      
      RETURN
      END


      REAL*8 FUNCTION FN_CA(t,Ca_m)

C#### Function: FN_CA
C###  Type: REAL*8
C###  Description:
C###    Calcium twitch function for use with the fading memory model

      IMPLICIT NONE
!     Parameter List
      REAL*8 t,Ca_m
!     Common Blocks
      REAL*8      Ca_max,Ca_tau
      COMMON /Ca/ Ca_max,Ca_tau  

      FN_CA=Ca_m*t/Ca_tau*DEXP(1.d0-t/Ca_tau)

      RETURN
      END


      REAL*8 FUNCTION FN_G(tt,dt,L,Lprev,Cb,z0,PHI,T,To,Load)

C#### Function: FN_G
C###  Type: REAL*8
C###  Description:
C###    G=T-Load, where T is calc.d from L  

      IMPLICIT NONE
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      REAL*8 tt,dt,L,Lprev,Cb,z0,PHI(3),T,To,Load
!     Common Blocks
      INTEGER     IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/  IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8      a,ALFA(3),AA(3)
      COMMON /XB/ a,ALFA,AA
!     Local Variables 
      REAL*8 FN_TM,Q,FN_TO,z,dPHI
         
      z  = z0 + dt*FN_TM(Cb,z0,L)                 !tropomyosin kinetics
           
      dPHI=(L-Lprev)/(ALFA(1)*dt)
     '     *DEXP(ALFA(1)*tt)*(1.d0-DEXP(-ALFA(1)*dt))
      Q  = AA(1)*DEXP(-ALFA(1)*tt)*(PHI(1)+dPHI)      !Xbridge kinetics

      To = FN_TO(L,z)                                 !isometric tension
      T  = To*(1.d0+a*Q)/(1.d0-Q)                     !tension

c      IF(IDOP.EQ.3) THEN
        IF(DEBUG) write(*,'('' L='',E12.3,'' z='',E12.3,'' Q='',E12.3,
     '    '' To='',E12.3,'' T='',E12.3,'' Load='',E12.3)') 
     '    L,z,Q,To,T,Load
c      ENDIF

      FN_G = T-Load

      RETURN
      END


      REAL*8 FUNCTION FN_Q(PHI,dt,EXTENSION_RATIO_VALUES)

C#### Function: FN_Q
C###  Type: REAL*8
C###  Description:
C###    Returns updated Q & PHI for current timestep dt, from fading 
C###    memory model.

      IMPLICIT NONE
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      REAL*4 EXTENSION_RATIO_VALUES(*)
      REAL*8 PHI(3),dt
!     Common Blocks
      INTEGER     IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/  IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8      a,ALFA(3),AA(3)
      COMMON /XB/ a,ALFA,AA
      REAL*8          MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      COMMON /MECH_T/ MECH_TIMESCALE,MECH_TSTART,MECH_TEND,MECH_DT,
     '                MECH_TABT,MECH_TPS,TP_mem
      INTEGER                     current_time_step,previous_time_step
      COMMON /TIME_VARIABLE_INFO/ current_time_step,previous_time_step
!     Local Variables
      REAL*8 dPHI !,Q,RELERR
      REAL*8 FM_INTEGRAND1

      ! 1st Fading Memory term
      dPHI=0.5d0*(FM_INTEGRAND1(1,0.d0,EXTENSION_RATIO_VALUES)+
     '  FM_INTEGRAND1(1,dt,EXTENSION_RATIO_VALUES))*dt
      PHI(1)=DEXP(-ALFA(1)*dt)*PHI(1)+dPHI
c      IF(IDOP.GE.2) THEN
        IF(DEBUG) WRITE(*,'(8X,'' dPHI='',E10.3,''  PHI(1)='',E15.8)') 
     '    dPHI,PHI(1)
c      ENDIF

      ! 2nd Fading Memory term
      dPHI=0.5d0*(FM_INTEGRAND1(2,0.d0,EXTENSION_RATIO_VALUES)+
     '    FM_INTEGRAND1(2,dt,EXTENSION_RATIO_VALUES))*dt
      PHI(2)=EXP(-ALFA(2)*dt)*PHI(2)+dPHI
c      IF(IDOP.GE.2) THEN
        IF(DEBUG) WRITE(*,'(8X,'' dPHI='',E10.3,''  PHI(2)='',E15.8)') 
     '    dPHI,PHI(2)
c      ENDIF

      ! 3rd Fading Memory term
      dPHI=0.5d0*(FM_INTEGRAND1(3,0.d0,EXTENSION_RATIO_VALUES)+
     '    FM_INTEGRAND1(3,dt,EXTENSION_RATIO_VALUES))*dt
      PHI(3)=EXP(-ALFA(3)*dt)*PHI(3)+dPHI
c      IF(IDOP.GE.2) THEN
        IF(DEBUG) WRITE(*,'(8X,'' dPHI='',E10.3,''  PHI(3)='',E15.8)') 
     '    dPHI,PHI(3)
c      ENDIF

      FN_Q = AA(1)*PHI(1)+AA(2)*PHI(2)+AA(3)*PHI(3)

      RETURN
      END


      REAL*8 FUNCTION FN_TN(Ca,Cb,T,To)

C#### Function: Fn_Tn
C###  Type: REAL*8
C###  Description:
C###    Troponin kinetics function for fading memory model

      IMPLICIT NONE
!     Parameter List
      REAL*8 Ca,Cb,T,To
!     Common Blocks
      REAL*8      Rho0,Rho1,Cb_max
      COMMON /Tn/ Rho0,Rho1,Cb_max
      REAL*8         Tref,beta0,beta1,beta2,gamma,Ca_scale
      COMMON /SS_To/ Tref,beta0,beta1,beta2,gamma,Ca_scale
!     Local Variables
      REAL*8 TTo

C *** DPN 21/01/98 - updating to HMT model equation
C      FN_TN=Rho0*Ca*(Cb_max-Cb)-Rho1*Cb
c      IF (DABS(To).LT.1.0d-10) THEN
c        TTo = 0.0d0
c      ELSE
c        TTo = T/To
c      ENDIF

      IF (DABS(To).LT.1.0d-10) THEN
        TTo = 0.0d0
      ELSE
        TTo = T/(gamma*To)
      ENDIF
      FN_TN=Rho0*Ca*(Cb_max-Cb)-Rho1*(1.0d0-TTo)*Cb

      RETURN
      END


      REAL*8 FUNCTION FN_TM(Cb1,z1,L)

C#### Function: FN_TM
C###  Type: REAL*8
C###  Description:
C###    Tropomyosin kinetics for fading memory model (dz/dt)

      IMPLICIT NONE
C      INCLUDE 'cell/cell00.cmn'
!     Parameter List
      REAL*8 Cb1,z1,L
!     Common Blocks
      REAL*8         Tref,beta0,beta1,beta2,gamma,Ca_scale
      COMMON /SS_To/ Tref,beta0,beta1,beta2,gamma,Ca_scale
      REAL*8         ALFA0,AA0,Tm_n_0,Tm_p50_0
      COMMON /Tm/    ALFA0,AA0,Tm_n_0,Tm_p50_0
      REAL*8      Rho0,Rho1,Cb_max
      COMMON /Tn/ Rho0,Rho1,Cb_max
c      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
c      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
c      REAL*8        mech_pCa,Ca_m,z,Cb,PHI(3),To_initial
c      COMMON /VARS/ mech_pCa,Ca_m,z,Cb,PHI,To_initial
!     Local Variables 
      REAL*8 C50,Tm_n,Tm_p50,Cb_norm

      Tm_n   =Tm_n_0  *(1.d0+beta1*(L-1.d0)) !length dependence for n
      Tm_p50 =Tm_p50_0*(1.d0+beta2*(L-1.d0)) !length dependence for p50
      C50=10**(6.d0-Tm_p50) !uM

C DPN 05/08/98 - scale dz/dt by normalised [Ca]b
      Cb_norm = Cb1/Cb_max
c       FN_TM=alfa0*((Cb1/C50)**Tm_n *(1.d0-z1) - z1)
      FN_TM=alfa0*(((Cb1/C50)*Cb_norm)**Tm_n *(1.d0-z1) - z1)

      RETURN
      END


      REAL*8 FUNCTION FN_TO(L,z)

C#### Function: FnTo
C###  Type: REAL*8
C###  Description:
C###    Tension-length-pCa relation for fading memory model

      IMPLICIT NONE
!     Parameter List
      REAL*8 L,z !,Cb
!     Common Blocks
      REAL*8         Tref,beta0,beta1,beta2,gamma,Ca_scale
      COMMON /SS_To/ Tref,beta0,beta1,beta2,gamma,Ca_scale
         
      FN_TO=Tref*(1.d0+beta0*(L-1.d0))*z

      RETURN
      END


      REAL*8 FUNCTION FN_TO_SS(L,Cb)

C#### Function: FN_TO_SS
C###  Type: REAL*8
C###  Description:
C###    Steady state tension-length-pCa relation for fading memory model

      IMPLICIT NONE
!     Parameter List
      REAL*8 L,Cb !,z
!     Common Blocks
      REAL*8         Tref,beta0,beta1,beta2,gamma,Ca_scale
      COMMON /SS_To/ Tref,beta0,beta1,beta2,gamma,Ca_scale
      REAL*8         alfa0,AA0,Tm_n_0,Tm_p50_0
      COMMON /Tm/    alfa0,AA0,Tm_n_0,Tm_p50_0
!     Local Variables 
      REAL*8 C50,n,p50
      REAL*8 FN_ZSS
         
      n   = Tm_n_0  *(1.d0+beta1*(L-1.d0)) !length dependence for n
      p50 = Tm_p50_0*(1.d0+beta2*(L-1.d0)) !length dependence for p50
      C50 = 10**(6.d0-p50) !uM

      FN_TO_SS=Tref*(1.d0+beta0*(L-1.d0))*FN_ZSS(Cb,C50,n)

      RETURN
      END


      REAL*8 FUNCTION FN_ZSS(Cb,C50,n)

C#### Function: FN_ZSS
C###  Type: REAL*8
C###  Description:
C###    Steady state z for fading memory model

      IMPLICIT NONE
!     Parameter List
      REAL*8 Cb,C50,n

      FN_ZSS= (Cb/C50)**n/(1.d0+(Cb/C50)**n)

      RETURN
      END


      SUBROUTINE GATES(T,Y,F)

C#### Subroutine: GATES
C###  Description:
C###    Computes time constants and activation variables.

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
C      INCLUDE 'cell/oxs001.cmn'
      INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs004.cmn'
!     Parameter List
      REAL*8 F(*),T,Y(*)
!     Local Variables
      REAL*8 EM
c dpn 07/05/98 - unused real*8's - CACH(10),ECH(10),EKC(0:11),IACHC(0:11),IBKC(0:11),ICAKC(0:11),IFKC(0:11),IFNAC(0:11),IKC(0:11),IK1C(0:11),IMKC(0:11),IMKSAV(0:11),IMNA,IPC(0:11),IREL,ISI,ITOC(0:11),ITOT,ITRAN,IUP,KCH(10),NACH(10),PUMPCH(10),TCH(10),YSAV(0:11)
!      write(*,'('' >>>call gates'')')

      PMODE=3
      PLOTT=.TRUE.
      EM=-100.0d0
      DO WHILE(EM.LT.40.0d0)
        Y(1)=EM
c        CALL DFN_RATES(T,Y,F)
        CALL DFN_RATES(Y,F)
        Z1=Y(5)/(SPEED(5)*ALPHA(5))
C    ETC ETC
      ENDDO

      RETURN
      END


      REAL*8 FUNCTION GG(k,z,p,q) 

C#### Function: GG
C###  Type: REAL*8
C###  Description:
C###    Calculation of G(k,z,p,q) (eq A3; Ma & Zahalak,1991).

      IMPLICIT NONE
!     Passed variables
      INTEGER k
      REAL*8 z,p,q
!     Common blocks
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8      PI
      COMMON /PI/ PI
!     Local variables
      !INTEGER IFAIL
      REAL*8 F0,F1,F2,F3
      REAL*8 ERRORC
      !REAL*8   S15ABF
      !EXTERNAL S15ABF 

C *** Replace NAG function with CMISS function
c      F0=S15ABF(z,IFAIL)
c      IF(IFAIL.NE.0) THEN
c        WRITE(*,'('' Failure in S15ABF: IFAIL='',I3)') IFAIL
c      ENDIF
C???DB.  Where does ERRORC come from ?
C      F0 = (1.0d0/2.0d0)*ERRORC(-z/DSQRT(2.0d0))
      IF(k.GE.1) THEN
        F1 = -DEXP(-z**2/2.0d0)/DSQRT(2.0d0*PI)
      ENDIF
      IF(k.GE.2) THEN
        F2 = F0+z*F1
      ENDIF
      IF(k.GE.3) THEN
        F3 = 2.0d0*F1+z*z*F1
      ENDIF

      IF(k.EQ.0) THEN
        GG = F0
      ELSE IF(k.EQ.1) THEN
        GG = p*F0 + q*F1
      ELSE IF(k.EQ.2) THEN
        GG = p*p*F0 + 2.0d0*p*q*F1 + q*q*F2
      ELSE IF(k.EQ.3) THEN
        GG = p*p*p*F0 + 3.0d0*p*p*q*F1 + 3.0d0*p*q*q*F2 + q*q*q*F3
      ENDIF
      
      RETURN
      END


      SUBROUTINE HH(T,Y,DY,AII,AIO,CONTROL,L_AII,L_AIO,L_ARI,L_ARO,
     '  L_CONTROL,L_MODEL,L_PARAMETERS,MODEL,NUMBER_EQN,ARI,ARO,
     '  PARAMETERS,ERR_CODE)

C#### Subroutine: HH
C###  Description:
C###    Computes RHS of Hodgkin-Huxley equations.
C###    Voltage is in mV & time in ms.
C###    ALPHA(1) is alpha_h
C###    ALPHA(2) is alpha_m
C###    ALPHA(3) is alpha_n
C###    BETA(1)  is beta_h
C###    BETA(2)  is beta_m
C###    BETA(3)  is beta_n

      IMPLICIT NONE

!     Parameters variables
      INTEGER L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,
     '  L_PARAMETERS,NUMBER_EQN
      INTEGER AII(L_AII),AIO(L_AIO),CONTROL(L_CONTROL),
     '  ERR_CODE,MODEL(L_MODEL)
      REAL*8 ARI(L_ARI),ARO(L_ARO),PARAMETERS(L_PARAMETERS),
     '  T,Y(NUMBER_EQN),DY(NUMBER_EQN)
!     Local Variables
      REAL*8 ALPHA(3),BETA(3),V,Vm,m,h,n

      V=Y(1)
      h=Y(2)
      m=Y(3)
      n=Y(4)

!      WRITE(*,'('' t='',E12.4,'' V='',E12.4,'' m='',F5.3,'
!     '  //''' h='',F5.3,'' n='',F5.3)') t,Y(1),Y(2),Y(3),Y(4)

      IF (HH_Stim_tv.EQ.HH_Stim_id) THEN !use time variable
        HH_Stimulus = ARI(HH_Stim_tv_current_position)
      ELSE
        IF(t.GT.HH_TPS.AND.
     +    t.LT.HH_TPS+HH_TP) THEN
c        Istim=100.0d0
          HH_Stimulus=HH_ISTIM
        ELSEIF(HH_TPS2.GT.HH_TPS.AND.
     +      t.GT.HH_TPS2.AND.
     +      t.LT.HH_TPS2+HH_TP) THEN
c        Istim=100.0d0
          HH_Stimulus=HH_ISTIM2
        ELSE
          HH_Stimulus=0.0d0
        ENDIF
      ENDIF
      
!      Vrest=-68.d0
      Vm=V-HH_Vrest
       
! Compute rate constants
      ALPHA(1)= 0.07d0*DEXP(-Vm/20.0d0)
      ALPHA(2)= 0.10d0*(25.0d0-Vm)/(DEXP(0.1d0*(25.0d0-Vm)) - 1.0d0)
      ALPHA(3)= 0.01d0*(10.0d0-Vm)/(DEXP(0.1d0*(10.0d0-Vm)) - 1.0d0)

      BETA(1) = 1.0d0/(DEXP(0.1d0*(30.0d0-Vm)) + 1.0d0)
      BETA(2) = 4.000d0*DEXP(-Vm/18.0d0)
      BETA(3) = 0.125d0*DEXP(-Vm/80.0d0)

      HH_alpha_h = ALPHA(1)
      HH_beta_h  = BETA(1)
      HH_alpha_m = ALPHA(2)
      HH_beta_m  = BETA(2)
      HH_alpha_n = ALPHA(3)
      HH_beta_n  = BETA(3)

! Compute o.d.e. RHSs 
      HH_INa   = HH_g_Na*m**3*h*(V-HH_VNa)
      HH_IK    = HH_g_K *n**4  *(V-HH_VK)
      HH_Ileak = HH_g_L*(V-HH_VL)
      
      DY(1) = (HH_Stimulus - (HH_INa + HH_IK + 
     +  HH_Ileak)) / HH_Cap
      DY(2) = ALPHA(1)*(1.d0-h) - BETA(1)*h
      DY(3) = ALPHA(2)*(1.d0-m) - BETA(2)*m
      DY(4) = ALPHA(3)*(1.d0-n) - BETA(3)*n

      RETURN
      END 


      SUBROUTINE HODGKIN_HUXLEY(AII,AIO,CONTROL,ERROR_TYPE,IFAIL,IWORK,
     '  L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_IWORK,L_MODEL,
     '  L_PARAMETERS,L_WORK,MAX_ITERS,MAX_ORDER,MODEL,
     '  ABS_ERR,ARI,ARO,MAX_STEP,PARAMETERS,REL_ERR,WORK,Y_FULL,
     '  EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL,ERROR_C)

C#### Subroutine: HODGKIN_HUXLEY
C###  Description:
C###    Solve Hodgkin-Huxley equations.
C###    Voltage is in mV & time in ms.

      IMPLICIT NONE

! Passed variables
      INTEGER L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_IWORK,L_MODEL,
     '  L_PARAMETERS,L_WORK
      INTEGER AII(L_AII),AIO(L_AIO),CONTROL(L_CONTROL),ERROR_TYPE,
     '  IFAIL,IWORK(L_IWORK),MAX_ITERS,MAX_ORDER,MODEL(L_MODEL)
      REAL*8 ABS_ERR,ARI(L_ARI),ARO(L_ARO),MAX_STEP,
     '  PARAMETERS(L_PARAMETERS),REL_ERR,WORK(L_WORK)
      REAL*4 Y_FULL(*)
      LOGICAL EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL
      INTEGER ERROR_C(*)
! Local variables
      INTEGER NO_DAT,NO_EQN,NO_FULL,NT_ADD,NO_ADD,NT_EQN,NT_DAT
      REAL*8 TCURRENT,TNEXT,Y_EQN(50),DY(50)
      REAL*8 R,T,F
c      REAL*8 alpha_h,beta_h,alpha_m,beta_m,alpha_n,beta_n
      CHARACTER ERROR_FORTRAN*(200)

      EXTERNAL HH

      write(*,'('' >>>call HODGKIN_HUXLEY'')')

! Set parameters
      NT_DAT  = AII(1)
      NT_EQN  = AII(2)
      NT_ADD  = AII(3)





      R=8.314d0
      T=HH_T+273.15d0
      F=96487.d0
      IF(DABS(HH_ic_Na).GT.1.d-12) THEN
        HH_VNa = 1000.d0*R*T/F * 
     +    DLOG(HH_ec_Na/HH_ic_Na) !in mV
      ELSE
        HH_VNa = 0.d0
      ENDIF
      IF(DABS(HH_ic_K).GT.1.d-12) THEN
        HH_VK = 1000.d0*R*T/F * 
     +    DLOG(HH_ec_K/HH_ic_K) !in mV
      ELSE
        HH_VK = 0.d0
      ENDIF

!      Sea Hare (Aplysia)
C      Vrest = 1000.d0*R*T/F*DLOG((ec_K+0.12d0*ec_Na+1.44d0*51.d0)/
C     '  (ic_K+0.12d0*ic_Na+1.44d0*485.d0))

!      Squid
      HH_Vrest = 1000.d0*R*T/F*
     +  DLOG((HH_ec_K+0.04d0*HH_ec_Na+
     +  0.45d0*52.d0)/(HH_ic_K+0.04d0*
     +  HH_ic_Na+0.45d0*560.d0))

      WRITE(*,'('' gNa  ='',F12.3,'' gK   ='',F12.3,'
     +  //''' gL   ='',F12.3,'' Cap  ='',F12.3)') 
     +  HH_g_Na,HH_g_K,
     +  HH_g_L,HH_Cap
      WRITE(*,'('' [Na]i='',F12.3,'' [Na]e='',F12.3,'
     +  //''' [K]i ='',F12.3,'' [K]e ='',F12.3)') 
     +  HH_ic_Na,HH_ec_Na,
     +  HH_ic_K,HH_ec_K
      WRITE(*,'('' VNa  ='',F12.3,'' VK   ='',F12.3,'
     +  //''' VL   ='',F12.3,'' Vrest='',F12.3)') 
     +  HH_VNa,HH_VK,HH_VL,HH_Vrest


      WRITE(*,'(/'' TIMESCALE='',E12.3,'' TSTART='',E12.3,'
     +  //''' TEND='',E12.3,'' DT='',E12.3,/,'' TABT='',E12.3,'
     +  //''' TPS='',E12.3,'' TP='',E12.3)')
     +  HH_TIMESCALE,HH_TSTART,HH_TEND,
     +  HH_DT,HH_TABT,
     +  HH_TPS,HH_TP

      HH_Stimulus = 0.0d0
      HH_INa      = 0.0d0
      HH_IK       = 0.0d0
      HH_Ileak    = 0.0d0
      
      Y_EQN(1)=HH_Vrest !Use computed resting potential as initial
      HH_alpha_h=0.07d0  
      HH_beta_h=1.d0/(DEXP(3.d0)+1.d0)
      HH_alpha_m=2.5d0/(DEXP(2.5d0)-1.d0)
      HH_beta_m=4.d0
      HH_alpha_n=0.1d0/(DEXP(1.d0)-1)
      HH_beta_n=0.125d0
      Y_EQN(2)=HH_alpha_h/(HH_alpha_h+HH_beta_h)
      Y_EQN(3)=HH_alpha_m/(HH_alpha_m+HH_beta_m)
      Y_EQN(4)=HH_alpha_n/(HH_alpha_n+HH_beta_n)
      WRITE(*,'(/'' V='',F10.3,''   h='',F10.3,''   m='',F10.3,'
     +  //'''   n='',F10.3,/,/)') (Y_EQN(NO_EQN),NO_EQN=1,NT_EQN)

CC *** Save initial y's and additional outputs
      NO_FULL=0
      DO NO_EQN=1,NT_EQN
        NO_FULL=NO_FULL+1
        Y_FULL(NO_FULL)=SNGL(Y_EQN(NO_EQN))
      ENDDO
      IF (NT_ADD.GT.0) THEN
        DO NO_ADD=1,NT_ADD
          NO_FULL=NO_FULL+1
          Y_FULL(NO_FULL)=SNGL(ARO(NO_ADD))
        ENDDO
      ENDIF

C *** Calculate the solution.  Print and save after each TABT
      TCURRENT=HH_TSTART
      IFAIL=1 !for 1st call to ADAMS_MOULTON
      NO_DAT=1
      DO WHILE ((IFAIL.LE.2).AND.(NO_DAT.LT.NT_DAT))
        TNEXT=TCURRENT+HH_TABT
        CALL ADAMS_MOULTON(AII,AIO,CONTROL,ERROR_TYPE,IFAIL,IWORK,
     '    L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_IWORK,L_MODEL,
     '    L_PARAMETERS,L_WORK,MAX_ITERS,MAX_ORDER,MODEL,NT_EQN,
     '    ABS_ERR,ARI,ARO,DY,MAX_STEP,PARAMETERS,REL_ERR,TCURRENT,
     '    TNEXT,WORK,Y_EQN,EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL,
     '    HH,ERROR_FORTRAN)
        IF(IFAIL.EQ.2) THEN
          DO NO_EQN=1,NT_EQN
            NO_FULL=NO_FULL+1
            Y_FULL(NO_FULL)=SNGL(Y_EQN(NO_EQN))
          ENDDO
          IF (NT_ADD.GT.0) THEN
            DO NO_ADD=1,NT_ADD
              NO_FULL=NO_FULL+1
              Y_FULL(NO_FULL)=SNGL(ARO(NO_ADD))
            ENDDO
          ENDIF
          TCURRENT=TNEXT
          HH_Stim_tv_current_position = HH_Stim_tv_current_position+1
          NO_DAT=NO_DAT+1
        ENDIF !iflag
      ENDDO !while iflag

      RETURN
      END 


      SUBROUTINE INTERP(NT_EQN,TEND,H,T,THIRD,YOUT,YIN,F,D1,DDIF)

C#### Subroutine: INTERP
C###  Description:
C###    <html><pre>
C###    This procedure interpolates the values yout according to
C###
C###          yout = y       +   integral p(t) dt
C###                  (n+1)       t(n+1)
C###
C###          where p(t) is third order adams corrector polynomial
C###    </pre></html>

      IMPLICIT NONE
!     Parameter List
      INTEGER NT_EQN
      REAL*8 D1(*),DDIF(*),F(*),H,T,TEND,THIRD,YIN(*),YOUT(*)
!     Local Variables
      INTEGER NO_EQN !,J
      REAL*8 HO202,HOUT

!      write(*,'('' >>>call interp'')')

      HOUT=TEND-T
      HO202=0.5d0*HOUT*HOUT
      DO NO_EQN=1,NT_EQN
        YOUT(NO_EQN)=YIN(NO_EQN)+HOUT*F(NO_EQN)+HO202*D1(NO_EQN)
     '    +(THIRD*HOUT*HOUT*HOUT+H*HO202)*DDIF(NO_EQN)
      ENDDO

      RETURN
      END


      SUBROUTINE JRW(t,y,F)

C#### Subroutine: JRW
C###  Description:
C###    compute Jafri-Rice-Winslow eqns

      IMPLICIT NONE

c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/cell00.cmn'
      INCLUDE 'cell/deoxs00.cmn'

 ! Passed variables
      REAL*8 t,y(*),F(*)

      REAL*8 ENa,EK,EK1,EKp,ECaN,ENaN
      COMMON /JRW_EP/ ENa,EK,EK1,EKp,ECaN,ENaN

      REAL*8 JRW_INa,JRW_ICa,JRW_IK,JRW_IK1,JRW_IKp,JRW_INaCa,JRW_INaK,
     '  JRW_InsCa,JRW_IpCa,JRW_ICaK,JRW_ICab,JRW_INab,
     '  JRW_InsNa,JRW_InsK,Jrel,Jleak,Jup,Jtr,Jxfer,Jtrpn
      COMMON /JRW_I/ JRW_INa,JRW_ICa,JRW_IK,JRW_IK1,JRW_IKp,JRW_INaCa,
     '  JRW_INaK,JRW_InsCa,JRW_IpCa,JRW_ICaK,JRW_ICab,JRW_INab,
     '  JRW_InsNa,JRW_InsK,Jrel,Jleak,Jup,Jtr,Jxfer,Jtrpn

      REAL*8 alfa_m,beta_m,alfa_h,alfa_j,beta_h,beta_j,
     '  alfa_x,beta_x,alfa_K1,beta_K1,alfa_Ca,beta_Ca
      COMMON /JRW_RATE/ alfa_m,beta_m,alfa_h,alfa_j,beta_h,beta_j,
     '  alfa_x,beta_x,alfa_K1,beta_K1,alfa_Ca,beta_Ca

      REAL*8           RTF
      COMMON /JRW_RTF/ RTF

      !troponin kinetics for coupled problems
      REAL*8      Rho0,Rho1,Cb_max
      COMMON /Tn/ Rho0,Rho1,Cb_max
      REAL*8         Tref,beta0,beta1,beta2,gamma,Ca_scale
      COMMON /SS_To/ Tref,beta0,beta1,beta2,gamma,Ca_scale

      REAL*8 Save_T,Save_To
      COMMON /COUPLING_TENSION/ Save_T,Save_to

      !local variables
      REAL*8 V,mNa,hNa,jNa,xK,CaSubSpace,CaJSR,CaNSR,HTRPNCa,
     '  LTRPNCa,PO1,PO2,PC1,PC2,yL,C0,C1,C2,C3,C4,O,Cca0,Cca1,Cca2,
     '  Cca3,Cca4,Oca,Bi,BSS,BJSR,
     '  y_inf,tau_y,alfa_Ca_dash,beta_Ca_dash,gama_Ca,JRW_STIMSIZE,TTo
      REAL*8 ALPHA(6),BETA(6)

 ! State variables
      V       = y( 1) !membrane potential
      mNa     = y( 7) !Na channel m gate
      hNa     = y( 6) !Na channel h gate
      jNa     = y( 8) !Na channel j gate
      xK      = y( 4) !K  channel x gate
      Nai     = y( 2) !intracellular [Na] 
      Ki      = y( 9) !intracellular [K]
      Cai     = y( 3) !intracellular [Ca]
      CaSubSpace = y(13) !subspace [Ca]
      CaJSR   = y(11) !junctional SR [Ca]
      CaNSR   = y(10) !network SR [Ca]
      HTRPNCa = y(31) !Tn binding: fraction of Ca bound to low  affinity site
      LTRPNCa = y(30) !Tn binding: fraction of Ca bound to high affinity site
      PO1     = y(15) !RyR channel, open state 1
      PO2     = y(16) !RyR channel, open state 2
      PC1     = y(14) !RyR channel, closed state 1
      PC2     = y(17) !RyR channel, closed state 2
      yL      = y( 5) !L-type Ca channel: voltage inactivation gate  
      C0      = y(18) !L-type Ca channel: mode normal,  closed state 0
      C1      = y(19) !L-type Ca channel: mode normal,  closed state 1
      C2      = y(20) !L-type Ca channel: mode normal,  closed state 2
      C3      = y(21) !L-type Ca channel: mode normal,  closed state 3
      C4      = y(22) !L-type Ca channel: mode normal,  closed state 4
      O       = y(23) !L-type Ca channel: mode normal,  open state 
      Cca0    = y(24) !L-type Ca channel: mode calcium, closed state 0
      Cca1    = y(25) !L-type Ca channel: mode calcium, closed state 1
      Cca2    = y(26) !L-type Ca channel: mode calcium, closed state 2
      Cca3    = y(27) !L-type Ca channel: mode calcium, closed state 3
      Cca4    = y(28) !L-type Ca channel: mode calcium, closed state 4
      Oca     = y(29) !L-type Ca channel: mode calcium, open state

      Cao     = y(12) !extracellular Ca

 ! Buffer scale factors
      Bi   = 1.d0/(1.d0+JRW_CMDN_tot*JRW_km_CMDN/(JRW_km_CMDN+Cai  )**2)
      BSS  = 1.d0/(1.d0+JRW_CMDN_tot*JRW_km_CMDN/
     '  (JRW_km_CMDN+CaSubSpace )**2)
      BJSR = 1.d0/(1.d0+JRW_CSQN_tot*JRW_km_CSQN/(JRW_km_CSQN+CaJSR)**2)
      
 ! Fluxes
      Jrel  = JRW_v1*(PO1+PO2)*(CaJSR-CaSubSpace) !RyR channel flux
      Jleak = JRW_v2*(CaNSR-Cai) !leak flux from NSR to myoplasm
      Jup   = JRW_v3*Cai**2/(JRW_km_up**2+Cai**2) !SR Ca-ATPase pump flux
      Jtr   = (CaNSR-CaJSR)/JRW_tau_tr !transfer flux from NSR to JSR
      Jxfer = (CaSubSpace-Cai)/JRW_tau_xfer !transfer flux from SS to myoplasm

C *** DPN 23/07/98 - evaluate dLTRPNCa/dt here to avoid repitition and
C ***                errors!!
C DPN 25/06/98
C *** If solving coupled problem, need to include tension dependence
C *** release.
      IF (MECHANICS_MODEL.EQ.2.AND.TENSION_DEPENDENT_RELEASE) THEN
        ! use HMT parameters for low-affinity troponin
        IF (DABS(Save_To).LT.1.0d-10) THEN
          TTo = 0.0d0
        ELSE
          TTo = Save_T/(gamma*Save_To)
        ENDIF
c        F(30)  =  Rho0*1.0d-3*Cai*(JRW_LTRPN_tot-LTRPNCa) !eqn 47
c     '    -Rho1*1.0d-3*(1.0d0-TTo)*LTRPNCa
        F(30)  =  JRW_kp_ltrpn*Cai*(JRW_LTRPN_tot-LTRPNCa) !eqn 47
     '    -JRW_km_ltrpn*(1.0d0-TTo)*LTRPNCa
      ELSE
        F(30)  =  JRW_kp_ltrpn*Cai*(JRW_LTRPN_tot-LTRPNCa) !eqn 47
     '    -JRW_km_ltrpn*LTRPNCa
      ENDIF
C DPN 25/06/98
C *** If solving coupled problem, need to include tension dependence
C *** release.
c      IF (MECHANICS_MODEL.EQ.2) THEN
c        ! use HMT parameters for low-affinity troponin
c        ! convert from s -> ms
c        IF (DABS(Save_To).LT.1.0d-10) THEN
c          TTo = 0.0d0
c        ELSE
c          TTo = Save_T/(gamma*Save_To)
c        ENDIF
c        Jtrpn = JRW_kp_htrpn*Cai*(JRW_HTRPN_tot-HTRPNCa)
c     '    - JRW_km_htrpn*HTRPNCa
c     '    + Rho0*1.0d-3*Cai*(JRW_LTRPN_tot-LTRPNCa)
c     '    - Rho1*1.0d-3*(1.0d0-TTo)*LTRPNCa
c      ELSE
c        Jtrpn = JRW_kp_htrpn*Cai*(JRW_HTRPN_tot-HTRPNCa)
c     '    - JRW_km_htrpn*HTRPNCa
c     '    + JRW_kp_ltrpn*Cai*(JRW_LTRPN_tot-LTRPNCa)
c     '    - JRW_km_ltrpn*LTRPNCa
c      ENDIF

      Jtrpn = JRW_kp_htrpn*Cai*(JRW_HTRPN_tot-HTRPNCa)
     '    - JRW_km_htrpn*HTRPNCa
     '  + F(30)

 ! Equilibrium potentials
c *** DPN 28/09/98 - move to avoid repeating code
      CALL JRW_EQUILIB_POTS(RTF,JRW_Nao,Nai,JRW_Ko,JRW_PNaK,Ki,Cao,Cai)
c      ENa  = RTF*DLOG(JRW_Nao/Nai) !Na channel
c      EK   = RTF*DLOG((JRW_Ko+JRW_PNaK*JRW_Nao)/(Ki+JRW_PNaK*Nai)) !K  channel
c      EK1  = RTF*DLOG(JRW_Ko/Ki) !K1 channel
c      EKp  = EK1 !Kp channel
c      ECaN = 0.5d0*RTF*DLOG(Cao/Cai) !Ca background
c      ENaN = ENa !Na background

C *** DPN 20/07/98 - move rates to JRW_RATES()
      CALL JRW_RATES(V,EK1,ALPHA,BETA)
      alfa_m = ALPHA(1)
      alfa_h = ALPHA(2)
      alfa_j = ALPHA(3)
      alfa_x = ALPHA(4)
      alfa_k1 = ALPHA(5)
      alfa_Ca = ALPHA(6)
      beta_m = BETA(1)
      beta_h = BETA(2)
      beta_j = BETA(3)
      beta_x = BETA(4)
      beta_k1 = BETA(5)
      beta_Ca = BETA(6)

      CALL JRW_CURRENTS(y) !ionic currents

C *** Set the stimulus size.
      CALL JRW_CHANGE(t,JRW_STIMSIZE)

 ![ 1] dV/dt                                                         
c dpn test      F( 1)  = (JRW_STIMSIZE + INa+ICa+IK+IK1+IKp+INaCa+INaK !eqn 39
c     '  +InsCa+IpCa+ICAK+ICab+INab)
      F( 1)  = -(JRW_STIMSIZE + ISWTCH(16)*JRW_INa
     '  +ISWTCH(5)*ISWTCH(8)*JRW_ICa
     '  +ISWTCH(13)*JRW_IK+ISWTCH(14)*JRW_IK1+ISWTCH(21)*JRW_IKp
     '  +ISWTCH(17)*JRW_INaCa+ISWTCH(18)*JRW_INaK
     '  +ISWTCH(22)*JRW_InsCa+ISWTCH(23)*JRW_IpCa
     '  +ISWTCH(5)*ISWTCH(9)*JRW_ICAK+ISWTCH(2)*JRW_ICab
     '  +ISWTCH(4)*JRW_INab) !eqn 39
 ![ 2] dm/dt                                                          
      F( 7)  = alfa_m*(1.d0-mNa)-beta_m*mNa !eqn 40
 ![ 3] dh/dt                                                          
      F( 6)  = alfa_h*(1.d0-hNa)-beta_h*hNa !eqn 41 
 ![ 4] dj/dt                                                          
      F( 8)  = alfa_j*(1.d0-jNa)-beta_j*jNa !eqn 42
 ![ 5] dx/dt                                                          
      F( 4)  = alfa_x*(1.d0-xK)-beta_x*xK !eqn 43
 ![ 6] dNai/dt                                                        
      F( 2)  = -(JRW_INa+JRW_INab+JRW_InsNa+3.d0*JRW_INaCa+
     '  3.d0*JRW_INaK)*JRW_Acap/(JRW_V_myo*JRW_Faraday) !eqn 44
 ![ 7] dKi/dt                                                         
c dpn      F( 9)  = -(IK+IK1+IKp+ICaK-2.d0*INaK) !eqn 45
c     '  *JRW_Acap/(JRW_V_myo*JRW_Faraday)
      F( 9)  = -(JRW_IK+JRW_IK1+JRW_IKp+JRW_InsK+JRW_ICaK-
     '  2.d0*JRW_INaK)*JRW_Acap/(JRW_V_myo*JRW_Faraday) !eqn 45
 ![ 8] dCai/dt                                                        
      F( 3)  = Bi*(Jleak+Jxfer-Jup-Jtrpn-(JRW_ICab-2.d0*JRW_INaCa+
     '  JRW_IpCa)*JRW_Acap/(2.d0*JRW_V_myo*JRW_Faraday)) ! eqn 13
 ![ 9] dCaSS/dt                                                       
c      F(13)  = BSS*(Jrel*JRW_V_JSR/JRW_V_SS-Jxfer*JRW_V_myo/JRW_V_SS
c     '  -JRW_ICa*JRW_Acap/(2.d0*JRW_V_myo*JRW_Faraday)) !eqn 14
      F(13)  = BSS*(Jrel*JRW_V_JSR/JRW_V_SS-Jxfer*JRW_V_myo/JRW_V_SS
     '  -JRW_ICa*JRW_Acap/(2.d0*JRW_V_SS*JRW_Faraday)) !eqn 14
 ![10] dCaJSR/dt                                                      
      F(11)  = BJSR*(Jtr-Jrel) !eqn 15
 ![11] dCaNSR/dt                                                      
      F(10)  = (Jup-Jleak)*JRW_V_myo/JRW_V_NSR !eqn 16
     '  -Jtr*JRW_V_JSR/JRW_V_NSR
 ![12] dHTRPNCa/dt
      F(31)  =  JRW_kp_htrpn*Cai*(JRW_HTRPN_tot-HTRPNCa) !eqn 46
     '  -JRW_km_htrpn*HTRPNCa
 ![13] dLTRPNCa/dt
C *** DPN 23/07/98 - move the calculation of dLTRPNCa/dt up to where
C ***                Jtrpn is evaluated.

 ![14] dPC1/dt                                                        
      F(14)  = -JRW_kap*CaSubSpace**JRW_n*PC1+JRW_kam*PO1 !eqn 89
 ![15] dPO1/dt                                                        
c dpn      F(15)  = -JRW_kap*CaSS**JRW_n*PC1-JRW_kam*PO1 !eqn 90
c     '  -JRW_kbp*CaSS**JRW_m*PO1+JRW_kbm*PO2
c     '  -JRW_kcp*PO1+JRW_kcm*PC2
      F(15)  = JRW_kap*CaSubSpace**JRW_n*PC1-JRW_kam*PO1 !eqn 90
     '  -JRW_kbp*CaSubSpace**JRW_m*PO1+JRW_kbm*PO2
     '  -JRW_kcp*PO1+JRW_kcm*PC2
 ![16] dPO2/dt                                                        
      F(16)  =  JRW_kbp*CaSubSpace**JRW_m*PO1-JRW_kbm*PO2 !eqn 91
 ![17] dPC2/dt                                                        
      F(17)  =  JRW_kcp*PO1-JRW_kcm*PC2 !eqn 92
 ![18] dy/dt                                                          
      y_inf  =  1.0d0/(1.d0+DEXP((V+55.d0)/7.5d0))                
     '  + 0.1d0/(1.d0+DEXP((-V+21.d0)/6.d0))
      tau_y  =  20.d0+600.d0/(1.d0+DEXP((V+30.d0)/9.5d0))
C ??? DPN      F(18)  =  (y_inf-yL)/JRW_tau_y !eqn 34
      F( 5)  =  (y_inf-yL)/tau_y !eqn 34
 ![19] dC0/dt                                                         
      alfa_Ca_dash = alfa_Ca*JRW_a
      beta_Ca_dash = beta_Ca/JRW_b
      gama_Ca      = 0.1875d0*CaSubSpace
c dpn      F(18)  = beta_Ca*C1+JRW_w*Cca0-(4.d0*alfa_Ca+gama_Ca)*Cao !eqn 22
      F(18)  = beta_Ca*C1+JRW_w*Cca0-(4.d0*alfa_Ca+gama_Ca)*C0 !eqn 22
 ![20] dC1/dt                                                         
      F(19)  = 4.d0*alfa_Ca*C0+2.d0*beta_Ca*C2+JRW_w/JRW_b *Cca1
     '  -(     beta_Ca+3.d0*alfa_Ca+gama_Ca*JRW_a   )*C1 !eqn 23
 ![21] dC2/dt                                                         
c      F(20)  = 3.d0*alfa_Ca*C1+3.d0*beta_Ca*C2+JRW_w/JRW_b**2*Cca2 !eqn 24
c     '  -(2.d0*beta_Ca+2.d0*alfa_Ca+gama_Ca*JRW_a**2)*C2
      F(20)  = 3.d0*alfa_Ca*C1+3.d0*beta_Ca*C3+JRW_w/JRW_b**2*Cca2
     '  -(2.d0*beta_Ca+2.d0*alfa_Ca+gama_Ca*JRW_a**2)*C2 !eqn 24
 ![22] dC3/dt                                                        
c dpn      F(21)  = 2.d0*alfa_Ca*C2+4.d0*beta_Ca*C2+JRW_w/JRW_b**3*Cca3 !eqn 25
c     '  -(3.d0*beta_Ca+     alfa_Ca+gama_Ca*JRW_a**3)*C3
      F(21)  = 2.d0*alfa_Ca*C2+4.d0*beta_Ca*C4+JRW_w/JRW_b**3*Cca3
     '  -(3.d0*beta_Ca+     alfa_Ca+gama_Ca*JRW_a**3)*C3 !eqn 25
 ![23] dC4/dt                                                         
      F(22)  =      alfa_Ca*C3+        JRW_g*O+JRW_w/JRW_b**4*Cca4
     '  -(4.d0*beta_Ca  + JRW_f  +  gama_Ca*JRW_a**4)*C4 !eqn 26
 ![24] dO/dt                                                          
      F(23)  = JRW_f*C4-JRW_g*O !eqn 27
 ![25] dCca0/dt                                                       
      F(24)  =      beta_Ca_dash*Cca1+gama_Ca*C0 !eqn 28
     '  -(4.d0*alfa_Ca_dash+JRW_w)*Cca0
 ![26] dCca1/dt                                                       
      F(25)  = 4.d0*alfa_Ca_dash*Cca0+2.d0*beta_Ca_dash*Cca2 !eqn 29
     '  +gama_Ca*JRW_a*C1
     '  -(     beta_Ca_dash     +3.d0*alfa_Ca_dash
     '  +JRW_w/JRW_b   )*Cca1
 ![27] dCca2/dt                                                       
      F(26)  = 3.d0*alfa_Ca_dash*Cca1+3.d0*beta_Ca_dash*Cca3 !eqn 30
     '  +gama_Ca*JRW_a**2*C2
     '  -(2.d0*beta_Ca_dash     +2.d0*alfa_Ca_dash
     '  +JRW_w/JRW_b**2)*Cca2
 ![28] dCca3/dt                                                       
      F(27)  = 2.d0*alfa_Ca_dash*Cca2+4.d0*beta_Ca_dash*Cca4 !eqn 31
     '  +gama_Ca*JRW_a**3*C3
     '  -(3.d0*beta_Ca_dash     +     alfa_Ca_dash
     '  +JRW_w/JRW_b**3)*Cca3
 ![29] dCca4/dt                                                       
      F(28)  =      alfa_Ca_dash*Cca3+JRW_g_dash*Oca !eqn 32
     '  +gama_Ca*JRW_a**4*C4
     '  -(4.d0*beta_Ca_dash     +JRW_f_dash
     '  +JRW_w/JRW_b**4)*Cca4
 ![30] dOca/dt                                                       
      F(29)  =        JRW_f_dash*Cca4-JRW_g_dash*Oca !eqn 33
 ![31] dCao/dt
      F(12)  = 0.0d0 !extra Ca constant.

      RETURN
      END


      SUBROUTINE JRW_CHANGE(t,JRW_STIMSIZE)

C#### Subroutine: JRW_CHANGE
C###  Description:
C###    Sets the stimulus size depending on the time
      
      IMPLICIT NONE

c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/cell00.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      !Passed variables
      REAL*8 t,JRW_STIMSIZE

      REAL*8 PERIOD !SMAR009 21/12/98 ,stimulation_time
c      REAL*8 fast,slow

c     fast = 1.5 !Hz
c     slow = 0.5 !Hz
      
C *** Set the stimulus curent
      IF (t.GE.TPS.AND.t.LE.TPS+TP) THEN
        JRW_STIMSIZE = JRW_Istim
      ELSE
        JRW_STIMSIZE = 0.0d0
      ENDIF

C *** Evaluate the new stimulus time
c      IF (t.LE.30.0d3) THEN
c        IF (t.GT.TPS+TP) THEN
c          PERIOD = 1.0d0 / (0.3d0*1.0d-3) !freq in Hz (1/s -> 1/ms)
c          TPS = TPS + PERIOD
c        ENDIF
c      ELSEIF (t.LE.80.0d3) THEN
c        IF (t.GT.TPS+TP) THEN
c          PERIOD = 1.0d0 / (0.1d0*1.0d-3) !freq in Hz (1/s -> 1/ms)
c          TPS = TPS + PERIOD
c        ENDIF
c      ELSEIF (t.LE.100.0d3) THEN
c        IF (t.GT.TPS+TP) THEN
c          PERIOD = 1.0d0 / (1.0d0*1.0d-3) !freq in Hz (1/s -> 1/ms)
c          TPS = TPS + PERIOD
c        ENDIF
c      ELSEIF (t.LE.150.0d3) THEN
c        IF (t.GT.TPS+TP) THEN
c          PERIOD = 1.0d0 / (0.1d0*1.0d-3) !freq in Hz (1/s -> 1/ms)
c          TPS = TPS + PERIOD
c        ENDIF
c      ELSEIF (t.LE.170.0d3) THEN
c        IF (t.GT.TPS+TP) THEN
c          PERIOD = 1.0d0 / (3.3d0*1.0d-3) !freq in Hz (1/s -> 1/ms)
c          TPS = TPS + PERIOD
c        ENDIF
c      ELSEIF (t.LE.200.0d3) THEN
c        IF (t.GT.TPS+TP) THEN
c          PERIOD = 1.0d0 / (0.2d0*1.0d-3) !freq in Hz (1/s -> 1/ms)
c          TPS = TPS + PERIOD
c        ENDIF
c      ENDIF

c      IF (t.LE.30.0d3) THEN
c        IF (t.GT.TPS+TP) THEN
c          PERIOD = 1.0d0 / (slow*1.0d-3) !freq in Hz (1/s -> 1/ms)
c          TPS = TPS + PERIOD
c        ENDIF
c      ELSE
c        TPS = 90.0d3
c      ENDIF


C *** Evaluate the new stimulus time
      IF(STIM_FREQ.GT.0.0d0) THEN
        IF (t.GT.TPS+TP) THEN
          PERIOD = 1.0d0 / (STIM_FREQ*1.0d-3) !freq in Hz (1/s -> 1/ms)
          TPS = TPS + PERIOD
        ENDIF
      ENDIF

      RETURN
      END


      SUBROUTINE JRW_CURRENTS(y)

C#### Subroutine: JRW_CURRENTS
C###  Description:
C###    Evaluate currents for JRW model.

      IMPLICIT NONE

      INCLUDE 'cell/cell00.cmn'

 ! Passed variables
      REAL*8 y(*)
 !Common blocks
      REAL*8 alfa_m,beta_m,alfa_h,alfa_j,beta_h,beta_j,
     '  alfa_x,beta_x,alfa_K1,beta_K1,alfa_Ca,beta_Ca
      COMMON /JRW_RATE/ alfa_m,beta_m,alfa_h,alfa_j,beta_h,beta_j,
     '  alfa_x,beta_x,alfa_K1,beta_K1,alfa_Ca,beta_Ca

      REAL*8 ENa,EK,EK1,EKp,ECaN,ENaN
      COMMON /JRW_EP/ ENa,EK,EK1,EKp,ECaN,ENaN

      REAL*8                    IINa,IIK,IIK1,IICa,IICaK,IICaNa
      COMMON /JRW_MAX_CURRENTS/ IINa,IIK,IIK1,IICa,IICaK,IICaNa

      REAL*8 JRW_INa,JRW_ICa,JRW_IK,JRW_IK1,JRW_IKp,JRW_INaCa,JRW_INaK,
     '  JRW_InsCa,JRW_IpCa,JRW_ICaK,JRW_ICab,JRW_INab,
     '  JRW_InsNa,JRW_InsK,Jrel,Jleak,Jup,Jtr,Jxfer,Jtrpn
      COMMON /JRW_I/ JRW_INa,JRW_ICa,JRW_IK,JRW_IK1,JRW_IKp,JRW_INaCa,
     '  JRW_INaK,JRW_InsCa,JRW_IpCa,JRW_ICaK,JRW_ICab,JRW_INab,
     '  JRW_InsNa,JRW_InsK,Jrel,Jleak,Jup,Jtr,Jxfer,Jtrpn

      REAL*8           RTF
      COMMON /JRW_RTF/ RTF

      !local variables
      REAL*8 V,mNa,hNa,jNa,xK,Nai,Ki,Cai,Cao,yL,O,Oca,VRTF,GK,Xi,GK1,
     '  K1inf,Kp,sigma,fNaK,InsNa_max,InsK_max,TVRTF,ICa_max,PK_dash

C DPN unused - ,C0,C1,C2,C3,C4,CaSS,CaJSR,CaNSR,Cca0,Cca1,Cca2,Cca3,Cca4,HTRPNCa,LTRPNCa,PO1,PO2,PC1,PC2

      V       = y( 1) !membrane potential
      mNa     = y( 7) !Na channel m gate
      hNa     = y( 6) !Na channel h gate
      jNa     = y( 8) !Na channel j gate
      xK      = y( 4) !K  channel x gate
      Nai     = y( 2) !intracellular [Na] 
      Ki      = y( 9) !intracellular [K]
      Cai     = y( 3) !intracellular [Ca]
c      CaSS    = y(13) !subspace [Ca]
c      CaJSR   = y(11) !junctional SR [Ca]
c      CaNSR   = y(10) !network SR [Ca]
c      HTRPNCa = y(31) !Tn binding: fraction of Ca bound to low  affinity site
c      LTRPNCa = y(30) !Tn binding: fraction of Ca bound to high affinity site
c      PO1     = y(15) !RyR channel, open state 1
c      PO2     = y(16) !RyR channel, open state 2
c      PC1     = y(14) !RyR channel, closed state 1
c      PC2     = y(17) !RyR channel, closed state 2
      yL      = y( 5) !L-type Ca channel: voltage inactivation gate  
c      C0      = y(18) !L-type Ca channel: mode normal,  closed state 0
c      C1      = y(19) !L-type Ca channel: mode normal,  closed state 1
c      C2      = y(20) !L-type Ca channel: mode normal,  closed state 2
c      C3      = y(21) !L-type Ca channel: mode normal,  closed state 3
c      C4      = y(22) !L-type Ca channel: mode normal,  closed state 4
      O       = y(23) !L-type Ca channel: mode normal,  open state 
c      Cca0    = y(24) !L-type Ca channel: mode calcium, closed state 0
c      Cca1    = y(25) !L-type Ca channel: mode calcium, closed state 1
c      Cca2    = y(26) !L-type Ca channel: mode calcium, closed state 2
c      Cca3    = y(27) !L-type Ca channel: mode calcium, closed state 3
c      Cca4    = y(28) !L-type Ca channel: mode calcium, closed state 4
      Oca     = y(29) !L-type Ca channel: mode calcium, open state
      Cao     = y(12) !extracellular Ca

      VRTF = V / RTF                        ! VF/RT (dimensionless)

 ! Na channel current                                                
      IINa = JRW_GNa_max*(V-ENa)
      JRW_INa   =  IINa * mNa*mNa*mNa*hNa*jNa !eqn 48

 ! K channel current                                                  
      GK    = 0.1128d0*DSQRT(JRW_Ko/5.4d0) !eqn 62
      Xi    = 1.d0/(1.d0+DEXP((V-56.26d0)/32.1d0)) !eqn 63
      IIK   = GK*(V-EK)
      JRW_IK    =  IIK * Xi*xK*xK !eqn 60

 ! K1 channel current
      GK1   = 0.75d0*DSQRT(JRW_Ko/5.4d0) !eqn 68
      K1inf = alfa_K1/(alfa_K1+beta_K1) !eqn 69
      IIK1  = GK1*(V-EK1)
      JRW_IK1   = IIK1 * K1inf !eqn 66

 ! Kp channel current
c dpn      Kp    = 1.d0/(1.d0+DEXP(7.488d0-V)/5.98d0) !eqn 74
      Kp    = 1.d0/(1.d0+DEXP((7.488d0-V)/5.98d0)) !eqn 74
      JRW_IKp   = JRW_GKp_max*Kp*(V-EKp) !eqn 72

 ! NaCa exchanger current
      JRW_INaCa = JRW_kNaCa*(DEXP(JRW_eta*VRTF)*Nai*Nai*Nai*Cao !eqn 75
     '  -DEXP((JRW_eta-1.d0)*VRTF)*JRW_Nao*JRW_Nao*JRW_Nao*Cai)
     '  /((JRW_km_Na**3+JRW_Nao**3)*(JRW_km_Ca+Cao)
     '  *(1.d0+JRW_ksat*DEXP((JRW_eta-1.d0)*VRTF)))

 ! NaK pump current
      sigma = (DEXP(JRW_Nao/67.3d0)-1.d0)/7.d0 !eqn 78
      fNaK  = 1.d0/(1.d0+0.1245d0*DEXP(-0.1d0*VRTF) !eqn 77
     '  +0.0365d0*sigma*DEXP(-VRTF))
      JRW_INaK  = JRW_INaK_max*fNaK*JRW_Ko/(JRW_Ko+JRW_km_Ko) !eqn 76
     '  /(1.d0+(JRW_km_Nai/Nai)**1.5d0)

 ! Nonspecific Ca-activated channel current
      InsNa_max = JRW_P_ns*VRTF*JRW_Faraday !eqn 81
     '  *0.75d0*(Nai*DEXP(VRTF)-JRW_Nao)/(DEXP(VRTF)-1.d0)
      JRW_InsNa     = InsNa_max/(1.d0+(JRW_km_ns/Cai)**3) !eqn 80
      InsK_max  = JRW_P_ns*VRTF*JRW_Faraday !eqn 83
     '  *0.75d0*(Ki*DEXP(VRTF)-JRW_Ko)/(DEXP(VRTF)-1.d0)
      JRW_InsK      = InsK_max/(1.d0+(JRW_km_ns/Cai)**3) !eqn 82
      JRW_InsCa     = JRW_InsNa+JRW_InsK !eqn 79

 ! Sarcolemmal Ca pump current
      JRW_IpCa      = JRW_ICap_max*Cai/(JRW_km_Cap+Cai) !eqn 84

 ! Ca background current
      JRW_ICab      = JRW_GCab_max*(V-ECaN) !eqn 85

 ! Na background current
      JRW_INab      = JRW_GNab_max*(V-ENaN) !eqn 87

 ! L-type Ca currents
      TVRTF     = 2.d0*VRTF                                         
      JRW_ICa   = JRW_PCa*yL*(O+OCa)*4.d0*VRTF*JRW_Faraday !eqn 37
     '  *(1.d-3*DEXP(TVRTF)-0.341d0*Cao)/(DEXP(TVRTF)-1.d0)
      ICa_max   = JRW_PCa*4.0d0*VRTF*JRW_Faraday*(0.001d0*DEXP(TVRTF)-
     '  0.341d0*Cao)/(DEXP(TVRTF)-1.0d0)
      IICa      = ICa_max
      PK_dash   = JRW_PK/(1.d0+ICa_max/JRW_ICa_half) !eqn 1
      IICaK     = PK_dash*VRTF*JRW_Faraday !eqn 38
     '  *(Ki*DEXP(VRTF)-JRW_Ko)/(DEXP(VRTF)-1.d0)
      JRW_ICaK      = IICaK * yL*(O+OCa)

      IICaNa = 0.0d0

      RETURN
      END


      SUBROUTINE JRW_CURRENTS_PLOT(NT_DAT,POTENTIALS,CURRENT,
     '  RESULTS,RPARAM)

C#### Subroutine: JRW_CURRENTS_PLOT
C###  Description:
C###    <html><pre>
C###    Calculates the specified current for computing the 
C###    steady state current
C###    CURRENT:- 1 = IICa   - Ca cpt of I(CaL) (maximum)
C###              2 = IICaK  - K cpt of I(CaL)
C###              3 = IICaNa - Na cpt of I(CaL)
C###              4 = IICa + IICaK + IICaNa
C###              6 = IIK
C###              7 = IINa
C###              8 = IIK1
C###    </pre></html>

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/cell00.cmn'
      INCLUDE 'cell/deoxs00.cmn'
!     Parameter List
      INTEGER NT_DAT,CURRENT
      REAL POTENTIALS(*),RESULTS(*)
      REAL*8 RPARAM(70)
!     Common blocks
      REAL*8        CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
      COMMON /conc/ CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
      REAL*8                    IINa,IIK,IIK1,IICa,IICaK,IICaNa
      COMMON /JRW_MAX_CURRENTS/ IINa,IIK,IIK1,IICa,IICaK,IICaNa
      REAL*8           RTF
      COMMON /JRW_RTF/ RTF
!     Local Variables
      INTEGER i
      REAL*8 Y(30) !SMAR009 21/12/98 ,t
      
C *** Initialise dummy arguments
c      t = 0.0d0
      DO i=1,30
        Y(i) = 0.0d0
      ENDDO
C *** Initialise required variables
      CALL DEFINE_JRW(RPARAM)
      Ki  = RPARAM(64)
      Nai = RPARAM(65)
      Cai = RPARAM(66)
      Cao = RPARAM(67)
      CALL JRW_EQUILIB_POTS(RTF,JRW_Nao,Nai,JRW_Ko,JRW_PNaK,Ki,Cao,Cai)
      
C *** Calculate the rate coefficients
      DO i=1,NT_DAT
        Y(1)  = DBLE(POTENTIALS(i)) ! V
        CALL JRW_CURRENTS(Y)
        IF (CURRENT.EQ.7) THEN
          RESULTS(i) = SNGL(IINa)
        ELSEIF (CURRENT.EQ.6) THEN
          RESULTS(i) = SNGL(IIK)
        ELSEIF (CURRENT.EQ.8) THEN
          RESULTS(i) = SNGL(IIK1)
        ELSEIF (CURRENT.EQ.1) THEN
          RESULTS(i) = SNGL(IICa)
        ELSEIF (CURRENT.EQ.2) THEN
          RESULTS(i) = SNGL(IICaK)
        ELSEIF (CURRENT.EQ.3) THEN
          RESULTS(i) = SNGL(IICaNa)
        ELSEIF (CURRENT.EQ.4) THEN
          RESULTS(i) = SNGL(IICa+IICaNa+IICaK)
        ENDIF
      ENDDO

      RETURN
      END


      SUBROUTINE JRW_EQUILIB_POTS(RTF,Nao,Nai,Ko,PNaK,Ki,Cao,Cai)

C#### Subroutine: JRW_EQUILIB_POTS
C###  Description:
C###    Calculates the equilibrium potentials for the JRW model

      IMPLICIT NONE
      
      !parameters
      REAL*8 RTF,Nao,Nai,Ko,PNaK,Ki,Cao,Cai
      !common blocks
      REAL*8 ENa,EK,EK1,EKp,ECaN,ENaN
      COMMON /JRW_EP/ ENa,EK,EK1,EKp,ECaN,ENaN

! Equilibrium potentials
      ENa  = RTF*DLOG(Nao/Nai) !Na channel
      EK   = RTF*DLOG((Ko+PNaK*Nao)/(Ki+PNaK*Nai)) !K  channel
      EK1  = RTF*DLOG(Ko/Ki) !K1 channel
      EKp  = EK1 !Kp channel
      ECaN = 0.5d0*RTF*DLOG(Cao/Cai) !Ca background
      ENaN = ENa !Na background

      RETURN
      END


      SUBROUTINE JRW_RATES(V,EK1,ALPHA,BETA)

C#### Subroutine: JRW_RATES
C###  Description:
C###    <html><pre>
C###    Calculates the rate coefficients for the JRW model.
C###    (1) = m }
C###    (2) = h } fast sodium gates
C###    (3) = j }
C###    (4) = X - K channel
C###    (5) = K1 - time independent K channel
C###    (6) = Ca - L-type calcium channel (d&f gates combined??)
C###    </pre></html>

      IMPLICIT NONE
      ! Passed Parameters
      REAL*8 V,EK1,ALPHA(6),BETA(6)

      ! Na channel gating rate constants
      ALPHA(1)  =  0.32d0*(V+47.13d0)/(1.d0-DEXP(-0.1d0*(V+47.13d0)))
      BETA(1)  = 0.08d0*DEXP(-V/11.d0)

      IF(V.GE.-40.d0) THEN
        ALPHA(2) = 0.d0
        ALPHA(3) = 0.d0
        BETA(2) = 1.d0/(0.13d0*(1.d0+DEXP(-(V+10.66d0)/11.1d0)))
        BETA(3) = 0.3d0*DEXP(-2.535d-7*V)
     '    /(1.d0+DEXP(-0.1d0*(V+32.d0)))
      ELSE
        ALPHA(2) = 0.135d0*DEXP(-(80.d0+V)/6.8d0)
        ALPHA(3) = (-1.27140d5*DEXP(0.2444d0*V)
     '    -3.474d-5*DEXP(-0.04391d0*V))
     '    *(V+37.78d0)/(1.d0+DEXP(0.311d0*(V+79.23d0)))
        BETA(2) = 3.56d0*DEXP(0.079d0*V)+3.1d5*DEXP(0.35d0*V)
        BETA(3) = 0.1212d0*DEXP(-0.01052d0*V)
     '    /(1.d0+DEXP(-0.1378d0*(V+40.14d0)))
      ENDIF !V

 ! K channel gating rate constants
      ALPHA(4)  = 7.19d-5*(V+30.d0)/( 1.d0-DEXP(-0.148d0*(V+30.d0)))
      BETA(4)  = 1.31d-4*(V+30.d0)/(-1.d0+DEXP(0.0687d0*(V+30.d0)))

 ! K1 channel gating rate constants
      ALPHA(5) = 1.02d0/(1.d0+DEXP(0.2385d0*(V-EK1-59.215d0)))
      BETA(5) = (0.4912d0*DEXP(0.08032d0*(V-EK1+5.476d0))
     '  +DEXP(0.06175d0*(V-EK1-594.31d0)))
     '  /(1.d0+DEXP(-0.5143d0*(V-EK1+4.753d0)))

 ! Ca channel gating rate constants
      ALPHA(6) = 0.4d0*DEXP((V+12.d0)/10.d0)
      BETA(6) = 0.05d0*DEXP(-(V+12.d0)/13.d0)

      RETURN
      END


      SUBROUTINE JRW_RATES_PLOT(KI_VALUES,NT_DAT,POTENTIALS,RATE,
     '  RESULTS,RPARAM)

C#### Subroutine: JRW_RATES_PLOT
C###  Description:
C###    Calculates the specified reaction rate coefficient pair 
C###    (alpha/beta)

      IMPLICIT NONE
!     Parameter List
      INTEGER NT_DAT,RATE
      REAL KI_VALUES(*),POTENTIALS(*),RESULTS(*),RPARAM(4)
!     Common blocks
      REAL*8           RTF
      COMMON /JRW_RTF/ RTF
!     Local Variables
      INTEGER i
      REAL*8 ALPHA(6),BETA(6),RPARAM1(70),Ko,EK1
      
C *** Initialise required variables
      Ko   = DBLE(RPARAM(1))
      RPARAM1(23) = DBLE(RPARAM(2)) !R
      RPARAM1(22) = DBLE(RPARAM(3)) !temp
      RPARAM1(21) = DBLE(RPARAM(4)) !F
      CALL DEFINE_JRW(RPARAM1)    !set RTF

C *** Calculate the rate coefficients
      DO i=1,NT_DAT
        !calculate EK1
        EK1  = RTF*DLOG(Ko/DBLE(KI_VALUES(i)))
        CALL JRW_RATES(DBLE(POTENTIALS(i)),EK1,ALPHA,BETA)
        RESULTS(i)        = SNGL(ALPHA(RATE)) !get the reaction
        RESULTS(i+NT_DAT) = SNGL(BETA(RATE))  !rate coefficients
      ENDDO

      RETURN
      END


      SUBROUTINE LIOXSPARA(Y)

C#### Subroutine: LIOXSPARA
C###  Description:
C###    Lists OXSOFT parameters.

      IMPLICIT NONE
      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
C      INCLUDE 'cell/oxs001.cmn'
      INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs004.cmn'
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      REAL*8 Y(*)
!     Local Variables
      INTEGER J !,I,IBEG,IEND
      CHARACTER FORMAT*600

!      write(*,'('' >>>call lioxspara'')')

      FORMAT='('' OXSOFT control mode parameters:'''
     '  //'/'' MODE    = '',I3,'' PMODE   = '',I3,'
     '  //'/'' PREP    = '',I3,'' SPACE   = '',I3,'
     '  //'/'' OUT     = '',I3,'' IDISP   = '',I3,'
     '  //'/'' DISP    = '',I3,'' CONTIN  = '',I3,'
     '  //'/'' SWITCH  = '',I3)'
      IF(DEBUG) 
     '  WRITE(*,FORMAT) MODE,PMODE,PREP,SPACE,OUT,IDISP,DISP,CONTIN,
     '  SWITCH
      
      FORMAT='(/'' OXSOFT model mode parameters:'''
     '  //'/'' KMODE   = '',I3,'' CAMODE  = '',I3,'
     '  //'/'' CANMODE = '',I3,'' NMODE   = '',I3,'
     '  //'/'' TOMODE  = '',I3,'' AMODE   = '',I3,'
     '  //'/'' RMODE   = '',I3,'' NAMODE  = '',I3,'
     '  //'/'' BMODE   = '',I3,'' SRMODE  = '',I3,'
     '  //'/'' CAOMODE = '',I3,'' MMODE   = '',I3,'
     '  //'/'' YMODE   = '',I3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  KMODE,CAMODE,CANMODE,NMODE,TOMODE,AMODE,RMODE,
     '  NAMODE,BMODE,SRMODE,CAOMODE,MMODE,YMODE
      FORMAT='('
     '  //'/'' CASS      = '',L1,'' GNASS     ='',L1,'
     '  //'/'' BUFFAST   = '',L1,'' SLPUMPMODE='',L1,'
     '  //'/'' CONTRACTMODE ='',L1)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  CASS,GNASS,BUFFAST,SLPUMPMODE,CONTRACTMODE
      FORMAT='('
     '  //'/'' CACT(1) = '',d11.3,'' MIK1    = '',d11.3,'
     '  //'/'' CACT(2) = '',d11.3,'' MIK     = '',d11.3,'
     '  //'/'' CACT(3) = '',d11.3,'' MIB     = '',d11.3,'
     '  //'/'' CACT(4) = '',d11.3,'' MITO    = '',d11.3,'
     '  //'/'' CACT(5) = '',d11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  CACT(1),MIK1,CACT(2),MIK,CACT(3),MIB,CACT(4),MITO,CACT(5)
      
      FORMAT='(/'' OXSOFT Ca_buffering parameters:'''
     '  //'/'' CTROP     = '',D11.3,'' MTROP     = '',D11.3,'
     '  //'/'' CALMOD    = '',D11.3,'' CAMGTROP  = '',D11.3,'
     '  //'/'' KCALON    = '',D11.3,'' KCALOFF   = '',D11.3,'
     '  //'/'' KTROCON   = '',D11.3,'' KTROCOFF  = '',D11.3,'
     '  //'/'' KTROMON   = '',D11.3,'' KTROMOFF  = '',D11.3,'
     '  //'/'' KTROCMON  = '',D11.3,'' KTROCMOFF = '',D11.3,'
     '  //'/'' KCACHOFF  = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  CTROP,MTROP,CALMOD,CAMGTROP,KCALON,KCALOFF,
     '  KTROCON,KTROCOFF,KTROMON,KTROMOFF,KTROCMON,KTROCMOFF,KCACHOFF
      
      FORMAT='(/'' OXSOFT Ca_sequestration parameters:'''
     '  //'/'' V12       = '',D11.3,'' V13       = '',D11.3,'
     '  //'/'' CA12M     = '',D11.3,'' VSURFCA   = '',D11.3,'
     '  //'/'' SRLEAK    = '',D11.3,'' KSLPUMP   = '',D11.3,'
     '  //'/'' KMSLPUMP  = '',D11.3,'' KCYCA     = '',D11.3,'
     '  //'/'' KSRCA     = '',D11.3,'' KXCS      = '',D11.3,'
     '  //'/'' TAUREL    = '',D11.3,'' NREL      = '',D11.3,'
     '  //'/'' CA12      = '',D11.3,'' CA13      = '',D11.3,'
     '  //'/'' REPRIM    = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  V12,V13,CA12M,VSURFCA,SRLEAK,KSLPUMP,KMSLPUMP,KCYCA,KSRCA,
     '  KXCS,TAUREL,NREL,CA12,CA13,REPRIM
     
      FORMAT='(/'' OXSOFT Concentration parameters:'''
     '  //'/'' KB        = '',D11.3,'' KI        = '',D11.3,'
     '  //'/'' NAI       = '',D11.3,'' CAI       = '',D11.3,'
     '  //'/'' CAO       = '',D11.3,'' CALIM     = '',D11.3,'
     '  //'/'' NAO       = '',D11.3,'' KNACA     = '',D11.3,'
     '  //'/'' KM        = '',D11.3,'' KMNA      = '',D11.3,'
     '  //'/'' KMK1      = '',D11.3,'' KMTO      = '',D11.3,'
     '  //'/'' KMF       = '',D11.3,'' KMCA      = '',D11.3,'
     '  //'/'' MG        = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  KB,KI,NAI,CAI,CAO,CALIM,NAO,KNACA,KM,KMNA,KMK1,KMTO,KMF,
     '  KMCA,MG

      FORMAT='(/'' OXSOFT Diffusion_space parameters:'''
     '  //'/'' PF        = '',D11.3,'' TORT      = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  PF,TORT
      
      FORMAT='(/'' OXSOFT Ion_channel parameters:'''
     '  //'/'' GNA       = '',D11.3,'' PCA       = '',D11.3,'
     '  //'/'' TAUF2     = '',D11.3,'' PCA3      = '',D11.3,'
     '  //'/'' IKM       = '',D11.3,'' GFNA      = '',D11.3,'
     '  //'/'' GFK       = '',D11.3,'' GTO       = '',D11.3,'
     '  //'/'' SHIFTTO   = '',D11.3,'' GB        = '',D11.3,'
     '  //'/'' GBNA      = '',D11.3,'' GBK       = '',D11.3,'
     '  //'/'' GBCA      = '',D11.3,'' GK1       = '',D11.3,'
     '  //'/'' SHIFTK1   = '',D11.3,'' GNAK      = '',D11.3,'
     '  //'/'' PCA2      = '',D11.3,'' PCAK      = '',D11.3,'
     '  //'/'' IACHM     = '',D11.3,'' ISCAL     = '',D11.3,'
     '  //'/'' PNAK      = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  GNA,PCA,TAUF2,PCA3,IKM,GFNA,GFK,GTO,SHIFTTO,GB,GBNA,GBK,
     '  GBCA,GK1,SHIFTK1,GNAK,PCA2,PCAK,IACHM,ISCAL,PNAK

      FORMAT='(/'' OXSOFT Other parameters:'''
     '  //'/'' C         = '',D11.3,'' EC        = '',D11.3,'
     '  //'/'' PULSESIZE = '',D11.3,'' IPULSESIZE= '',D11.3,'
     '  //'/'' ON        = '',D11.3,'' OFF       = '',D11.3,'
     '  //'/'' REP       = '',D11.3,'' STIMSIZE  = '',D11.3,'
     '  //'/'' VOL       = '',D11.3,'' RS        = '',D11.3,'
     '  //'/'' RE        = '',D11.3,'' AMP       = '',D11.3,'
     '  //'/'' DX        = '',D11.3,'' NINTERVAL = '',D11.3,'
     '  //'/'' PREPLENGTH= '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  C,EC,PULSESIZE,IPULSESIZE,ON,OFF,REP,STIMSIZE,VOL,RS,RE,
     '  AMP,DX,NINTERVAL,PREPLENGTH

      FORMAT='(/'' OXSOFT Pump_exchange parameters:'''
     '  //'/'' PUMP      = '',D11.3,'' NNAK      = '',D11.3,'
     '  //'/'' INACAM    = '',D11.3,'' NNACA     = '',D11.3,'
     '  //'/'' SNACA     = '',D11.3,'' DNACA     = '',D11.3,'
     '  //'/'' YNACA     = '',D11.3,'' SCALE     = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  PUMP,NNAK,INACAM,NNACA,SNACA,DNACA,YNACA,SCALE
      
      FORMAT='(/'' OXSOFT Single_cell parameters:'''
     '  //'/'' NAPE      = '',D11.3,'' CAPE      = '',D11.3,'
     '  //'/'' KPE       = '',D11.3,'' TNAP      = '',D11.3,'
     '  //'/'' TCAP      = '',D11.3,'' TKP       = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  NAPE,CAPE,KPE,TNAP,TCAP,TKP
      
      FORMAT='(/'' OXSOFT Time_step parameters:'''
     '  //'/'' TIMESCALE = '',D11.3,'' TSTART    = '',D11.3,'
     '  //'/'' TEND      = '',D11.3,'' DT        = '',D11.3,'
     '  //'/'' TABT      = '',D11.3,'' TPS       = '',D11.3,'
     '  //'/'' TP        = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT)
     '  TIMESCALE,TSTART,TEND,DT,TABT,TPS,TP
      
      FORMAT='(/'' OXSOFT miscellaneous parameters:'''
     '  //'/'' DVTEST = '',D11.3,'' DIFFCA = '',D11.3)'
      IF(DEBUG) WRITE(*,FORMAT) DVTEST,DIFFCA
      
      FORMAT='(/'' OXSOFT ALPHA parameters:'''
     '  //'/(10D11.3))'
      IF(DEBUG) WRITE(*,FORMAT) (ALPHA(J),J=1,33)
      
      FORMAT='(/'' OXSOFT BETA parameters:'''
     '  //'/(10D11.3))'
      IF(DEBUG) WRITE(*,FORMAT) (BETA(J),J=1,33)

      FORMAT='(/'' OXSOFT SHIFT parameters:'''
     '  //'/(10D11.3))'
      IF(DEBUG) WRITE(*,FORMAT) (SHIFT(J),J=1,33)
      
      FORMAT='(/'' OXSOFT SPEED parameters:'''
     '  //'/(10D11.3))'
      IF(DEBUG) WRITE(*,FORMAT) (SPEED(J),J=1,33)
      
      FORMAT='(/'' OXSOFT y-variables:'''
     '  //'/(10D11.3))'
      IF(DEBUG) WRITE(*,FORMAT) (Y(J),J=1,33)

      RETURN
      END


c      SUBROUTINE LR(t,Y,Func)

      SUBROUTINE LR(T,Y,DY,AII,AIO,CONTROL,L_AII,L_AIO,L_ARI,L_ARO,
     '  L_CONTROL,L_MODEL,L_PARAMETERS,MODEL,NUMBER_EQN,ARI,ARO,
     '  PARAMETERS,ERR_CODE)

C#### Subroutine: LR
C###  Description:
C###    Computes RHS of Luo-Rudy equations.
C###    Voltage is in mV & time in ms.

      IMPLICIT NONE
!     Parameter List
      INTEGER L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_MODEL,
     '  L_PARAMETERS,NUMBER_EQN
      INTEGER AII(L_AII),AIO(L_AIO),CONTROL(L_CONTROL),
     '  ERR_CODE,MODEL(L_MODEL)
      REAL*8 ARI(L_ARI),ARO(L_ARO),PARAMETERS(L_PARAMETERS),
     '  T,Y(NUMBER_EQN),DY(NUMBER_EQN)
!     Common blocks
c      REAL*8        I_Na,I_Ca,I_K,F_NSR,F_JSR
c      COMMON /isum/ I_Na,I_Ca,I_K,F_NSR,F_JSR
cp      REAL*8        TRPN,CMDN,CSQN
cp      COMMON /buff/ TRPN,CMDN,CSQN
      REAL*8        I(19),ICa !DPN 3/10/97 - extending I()
      COMMON /curr/ I,ICa
!     Local Variables
      REAL*8 VRatio_MJ, VRatio_MN, flux
      REAL*8 ALFA(7),BETA(7),I_stim,m,h,j,d,f,X !,Cm,V
      REAL*8 Iv_temp,RTONF
      REAL*8 LR_I(19),LR_ICa
      INTEGER count,ISWTCH(50)
      LOGICAL SINGLE_CELL

      SINGLE_CELL = .TRUE.

      RTONF = (LR_Temp+273.d0)*86.16d-3 !mV

      DO count=1,50
        ISWTCH(count)=1
      ENDDO
! Assign names to variables
C dpn 17/02/98      V = Y( 1)
      !?? DPN 3/10/97 - wrong indexing ??
      m = Y(8)     !Y( 2)
      h = Y(7)     !Y( 3)
      j = Y(9)     !Y( 4)
      d = Y(5)     !Y( 5)
      f = Y(6)     !Y( 6)
      X = Y(4)     !Y( 7)

      VRatio_MJ = LR_VolJSR/LR_VolMyo
      VRatio_MN = LR_VolNSR/LR_VolMyo

! Setting the stimulus current between t=5.0 and t=5.5ms
C??? DPN 7/10/97 - remove hard-coded times
C      IF ((t.GE.Tstim).AND.(t.LE.Tstim+0.5d0)) THEN
C        I_stim=-1.d2
      IF ((t.GE.LR_TPS).AND.
     +  (t.LE.LR_TPS+LR_TP)) THEN
        LR_Stimulus=LR_Istim
      ELSE
        IF (t.GT.LR_TPS+LR_TP) THEN
          IF (LR_Number_of_stimulii.LT.LR_numStim) THEN
            LR_TPS = LR_TPS + 
     +        LR_dtp
            LR_Number_of_stimulii = LR_Number_of_stimulii + 1
          ENDIF
        ENDIF
        LR_Stimulus=0.d0
      ENDIF
       
! Compute rate constants
      CALL L_R_RATES(Y,ALFA,BETA,RTONF,PARAMETERS,L_PARAMETERS)

! Compute ionic currents
      CALL LR_CURRENTS(t,Y,ALFA,BETA,RTONF,ARO,L_ARO,LR_ICa,
     '  PARAMETERS,L_PARAMETERS)
c      IF (SINGLE_CELL) THEN
c        DO count=1,19
c          I(count) = LR_I(count)
c        ENDDO
c        ICa = LR_ICa
c        I_Na = ISUMS(1)
c        I_Ca = ISUMS(2)
c        I_K = ISUMS(3)
c        F_NSR = ISUMS(4)
c        F_JSR = ISUMS(5)
c      ENDIF

      flux = LR_Irel*VRatio_MJ+
     +  (-LR_Iup+LR_Ileak)*VRatio_MN      

! Compute o.d.e. RHS 
C *** DPN 15/04/98 - adding switch to each current in the total
      Iv_temp = ISWTCH(14)*LR_IK1+ISWTCH(21)*LR_IKp+
     +  ISWTCH(18)*LR_INaK
     +  +ISWTCH(26)*LR_IpCa+ISWTCH(2)*LR_ICab+
     +  ISWTCH(4)*LR_INab
      Iv_temp = ISWTCH(24)*Iv_temp ! turns all time indep. currents off
      DY( 1) =  -(LR_Stimulus+ISWTCH(16)*LR_INa+
     +  ISWTCH(5)*LR_ICaL
     +  +ISWTCH(13)*LR_IK+ISWTCH(17)*LR_INaCa+
     +  ISWTCH(22)*LR_InsCa
     +  +Iv_temp)/LR_Cm
      DY( 8) =  ALFA(1)*(1.d0-m) - BETA(1)*m
      DY( 7) =  ALFA(2)*(1.d0-h) - BETA(2)*h
      DY( 9) =  ALFA(3)*(1.d0-j) - BETA(3)*j
      DY( 5) =  ALFA(4)*(1.d0-d) - BETA(4)*d
      DY( 6) =  ALFA(5)*(1.d0-f) - BETA(5)*f
      DY( 4) =  ALFA(6)*(1.d0-X) - BETA(6)*X
      DY( 2) =  -(LR_I_Na*LR_Acap)/
     +  (LR_VolMyo*LR_Farad)
      DY( 3) =  -(LR_I_Ca*LR_Acap)/
     +  (LR_VolMyo*2.d0*LR_Farad)+flux
      DY(10) =  -(LR_I_K*LR_Acap) /
     +  (LR_VolMyo*LR_Farad)
      DY(12) =  -LR_F_JSR
      DY(11) =  -LR_F_NSR
      DY(13) =  -((LR_ICa+LR_IpCa+LR_ICab)*1.d-3)/2.d0
      DY(14) = 0.0d0 ![Ca]o constant

      RETURN
      END 


      SUBROUTINE LR_CURRENTS(t,Y,ALFA,BETA,RTONF,ARO,L_ARO,ICa,
     '  PARAMETERS,L_PARAMETERS)

C#### Subroutine: LR_CURRENTS
C###  Description:
C###    Computes ionic currents for Luo-Rudy model

C??? DPN 3/10/97 - wrong indexing of Y array ???

C**** Potentials, gating variables, internal concentrations
C**** Y( 1) = V                                             Y( 1)
C**** Y( 2) = m   (Na V- & t- dep. activation gate)         Y( 8)
C**** Y( 3) = h   (Na fast V- & t- dep. inactivation gate)  Y( 7)
C**** Y( 4) = j   (Na slow V- & t- dep. inactivation gate)  Y( 9)
C**** Y( 5) = d   (Ca V- & t- dep. activation gate)         Y( 5)
C**** Y( 6) = f   (Ca V- & t- dep. inactivation gate)       Y( 6)
C****         fCa (Ca Ca-dep. inactivation gate)    
C**** Y( 7) = X   (K  V- & t- dep. activation gate)         Y( 4)
C****         Xi  (K  V-dep. inactivation gate)
C**** Y( 8) = internal Na concentration                     Y( 2)
C**** Y( 9) = internal Ca concentration                     Y( 3)
C**** Y(10) = internal K concentration                      Y(10)
C**** Y(11) = CaJSR concentration                           Y(12)
C**** Y(12) = CaNSR concentration                           Y(11)
C**** Y(13) = internal Ca concentration at T tubules due to 
C****         L-type channels                               Y(13)
C**** Y(14) = extracellular [Ca] (constant???)

C**** Currents (uA/uF)
C**** I( 1) = INa
C**** I( 2) = ICa_t
C**** I( 3) = IK
C**** I( 4) = IK1
C**** I( 5) = IKp
C**** I( 6) = INaCa
C**** I( 7) = INaK
C**** I( 8) = InsCa
C**** I( 9) = IpCa
C**** I(10) = ICab
C**** I(11) = INab
C**** I(12) = Iv
C**** I(13) = Irel (flux)
C**** I(14) = Iup (flux)
C**** I(15) = Ileak (flux)
C**** I(16) = Itr (flux)
C??? DPN 3/10/97 - extend I() to include ICa, ICaK, and ICaNa (cpts of 
C                  I(Ca(L)) = ICa_t)
C**** I(17) = ICa
C**** I(18) = ICaK
C**** I(19) = ICaNa

C**** total current for each ions
C**** I_Na  = sum of currents containing Na ions
C**** I_Ca  = sum of currents containing Ca ions
C**** I_K   = sum of currents containing K ions
C**** F_JSR = sum of fluxes moving into JSR
C**** F_NSR = sum of fluxes moving into NSR

      IMPLICIT NONE
!     Parameter List
      INTEGER L_ARO,L_PARAMETERS
      REAL*8 ARO(L_ARO),PARAMETERS(L_PARAMETERS)
      REAL*8 t,Y(*),ALFA(*),BETA(*),RTONF,I(19),ISUMS(5),ICa
!     Common blocks
      REAL*8        Cai_on,Cai2
      COMMON /Cai_/ Cai_on,Cai2

      REAL*8                   IICa,IICaK,IICaNa,IINa,IIK,IIK1,fca
      COMMON /LR_MAX_CURRENTS/ IICa,IICaK,IICaNa,IINa,IIK,IIK1,fca

!     Local Variables
      REAL*8 V,m,h,j,d,f,X
      REAL*8 ENa!,GGNa
      REAL*8 ICaK,ICaNa
      REAL*8 EK,GGK,Xi!,PNaK
      REAL*8 EK1,GGK1,K10
      REAL*8 EKp,Kp
      REAL*8 fNaK,sigma
      REAL*8 EnsCa,Vns,InsNa,IInsNa,InsK,IInsK
      REAL*8 ECaN!, GGCab
      REAL*8 ENaN!, GGNab
      REAL*8 CaJSRbuffer !!SMAR009 23/12/98 Cabuffer,
      REAL*8 Grel,GGrel
      REAL*8 t_CICR
      REAL*8 Kleak
      REAL*8 VRatio_JN
      REAL*8 LR_Cai,LR_Nai,LR_Ki,LR_Cao
      REAL*8       CSQN !SMAR009 21/12/98 ,TRPN,CMDN
      REAL*8        LR_CaNSR,LR_CaJSR
      REAL*8        KmCa
      LOGICAL DEBUG

      DEBUG = .FALSE.

! Initialising the volume parameters
      !VolJSR = 0.182d-6    !uL
      !VolNSR = 2.098d-6    !uL
      VRatio_JN = LR_VolNSR/LR_VolJSR

! Initialising the thermodynamic parameters
      V     = Y(1)                    !membrane potential (mV)
      !Temp  = 37.d0                   !temperature (deg C)
      !RTONF = (Temp+273.d0)*86.16d-3  !RT/F (mV)
      !Farad = 96.5d3                  !C/mol

! Standard ionic concentrations in mmol/L
      LR_Nai   = Y(2)   !Y(8)       ![Na]i
      !Nao   = 140.d0     ![Na]o
      LR_Cai   = Y(3)   !Y(9)       ![Ca]i
      !Cao   = 1.8d0      ![Ca]o
      LR_Ki    = Y(10)      ![K]i
      LR_Cao  = Y(14)
 

! Ca buffers in the myoplasm
      !??? DPN - move to common block ???
      !TRPNTRPN = 70.d-3               !mmol/L
      !CMDNCMDN = 50.d-3               !mmol/L
      !KmTRPN   = 0.5d-3               !mmol/L
      !KmCMDN   = 2.38d-3              !mmol/L
c      TRPN     = TRPNTRPN*(LR_Cai/(LR_Cai+KmTRPN))
c      CMDN     = CMDNCMDN*(LR_Cai/(LR_Cai+KmCMDN))


! Calcium concentration after taking the buffer TRPN and CMDN into 
! account
C dpn 03/06/98 - uncomment
!      Cabuffer = Cai-TRPN-CMDN
!      IF(t.GE.Tstim)THEN
!        IF(Cabuffer.GT.1.d-6)THEN
!          Cai = Cabuffer
!        ELSE 
!          Cai = 1.d-6
!        ENDIF
!      ENDIF
c      Cabuffer = Cai-TRPN-CMDN
c      IF(t.GE.TPS)THEN
c        IF(Cabuffer.GT.1.d-6)THEN
c          Cai = Cabuffer
c        ELSE
c          Cai = 1.d-6
c        ENDIF
c      ENDIF
C dpn 03/06/98

! Ca fluxes in the sarcoplasmic reticulum
      LR_CaJSR = Y(12)  !Y(11)                   !mmol/L
      LR_CaNSR = Y(11)  !Y(12)                   !mmol/L


! Ca buffer in JSR and CSQN
      !??? DPN - move to common block ???
      !KmCSQN    = 0.8d0               !mmol/L
      !CSQNCSQN  = 10.d0               !mmol/L
      CSQN      = LR_CSQNCSQN*(LR_CaJSR/
     +  (LR_CaJSR+LR_KmCSQN))


! Ca flux in the SR after taking CSQN buffering into account 
C dpn 03/06/98 - uncomment
!      CaJSRbuffer = CaJSR-CSQN
!      IF(t.GE.Tstim)THEN
!        IF(CaJSRbuffer.GT.1.d-3)THEN
!          CaJSR = CaJSRbuffer
!        ELSE
!          CaJSR = 1.d-3
!        ENDIF
!      ENDIF
      CaJSRbuffer = LR_CaJSR-CSQN
      IF(t.GE.LR_TPS)THEN
        IF(CaJSRbuffer.GT.1.d-3)THEN
          LR_CaJSR = CaJSRbuffer
        ELSE
          LR_CaJSR = 1.d-3
        ENDIF
      ENDIF
C dpn 03/06/98

! Ionic currents in the sarcolemma
! Fast Na current INa
      m = Y(8)  !Y(2)
      h = Y(7)  !Y(3)
      j = Y(9)  !Y(4)

      !GGNa = 16.d0                    !Na conductance (millisiemens/uF)
      ENa  = RTONF*dlog(LR_Nao/LR_Nai) 
 !Na reversal potential (mV)  
C *** DPN 29 April 1998 - grab max INa
c      I(1) = GGNa*m*m*m*h*j*(V-ENa)
      IINa = LR_GGNa * (V-ENa)
      LR_INa = m*m*m * h * j * IINa

! L-type Ca current ICa_t
      d = Y(5)
      f = Y(6)

      KmCa = LR_KmCa_L !0.6d-3                   !mmol/L
      fCa  = 1.d0/(1.d0+(LR_Cai/
     +  KmCa)**2) !Ca-dep. inact.n gate of L-type Ca

      !gCai = 1.d0                     !activity coefficent of Cai
      !gCao = 0.341d0                  !activity coefficent of Cao
      IICa = LR_PCa*4.d0*(V/RTONF)*LR_Farad*
     +  (LR_gCai*LR_Cai*dexp(2.d0*V/RTONF)
     +  -LR_gCao*LR_Cao)/(dexp(2.d0*V/RTONF)-1.d0)

      !gNai   = 0.75d0                 !activity coefficent of Nai
      !gNao   = 0.75d0                 !activity coefficent of Nao
      IICaNa = LR_PNa*(V/RTONF)*LR_Farad*
     +  (LR_gNai*LR_Nai*dexp(V/RTONF)-
     +  LR_gNao*LR_Nao)
     +  /(dexp(V/RTONF)-1.d0)

      !gKi    = 0.75d0                 !activity coefficent of Ki
      !gKo    = 0.75d0                 !activity coefficent of Ko
      IICaK  = LR_PK*(V/RTONF)*LR_Farad*
     +  (LR_gKi*LR_Ki*
     +  dexp(V/RTONF)-LR_gKo*LR_Ko)
     +  /(dexp(V/RTONF)-1.d0)

      ICa   = d*f*fCa*IICa
      ICaK  = d*f*fCa*IICaK
      ICaNa = d*f*fCa*IICaNa
      
      ! DPN 3/10/97 - include ICa,ICaK,ICaNa in I()
      LR_ICaLCa = ICa
      LR_ICaLK  = ICaK
      LR_ICaLNa = ICaNa

      LR_ICaL   = ICa + ICaK + ICaNa

! Time-dep. K current IK
      X    = Y(4)  !Y(7)!V- & t- dep. K-activation gate
      Xi   = 1.d0/(1.d0+dexp((V-56.26d0)/32.1d0)) 
                                      !V-dep. K-inactivation gate
      GGK  = 0.282d0*dsqrt(LR_Ko/5.4d0)  !millisiemens/uF
      !PNaK = 0.01833d0                !permeablity ratio of Na:K
      EK   = RTONF*dlog((LR_Ko+
     +  LR_PNaK*LR_Nao)/
     +  (LR_Ki+LR_PNaK*LR_Nai)) 
                                      !K reversal potential
C *** DPN 29 April 1998 - grab the max I(K)
c      I(3) = GGK*Xi*X*X*(V-EK)
      IIK = GGK*(V-EK)
      LR_IK = IIK * Xi * X * X

! Time-indep. K current IK1
      GGK1 = 0.75d0*dsqrt(LR_Ko/5.4d0)   !millisiemens/uF
      EK1  = RTONF*dlog(LR_Ko/LR_Ki)
      K10  = ALFA(7)/(ALFA(7)+BETA(7))
C *** DPN 29 April 1998 - grab max I(K1)
c      I(4) = GGK1*K10*(V-EK1)
      IIK1 = GGK1 * (V - EK1)
      LR_IK1 = IIK1 * K10

! Plateau K current IKp
      !GGKp = 0.0183d0                !millisiemens/uF
      EKp  = EK1                      !Kp reversal potential  
      Kp   = 1.d0/(1.d0+dexp((7.488d0-V)/5.98d0))
      LR_IKp = LR_GGKp*Kp*(V-EKp)

! Na-Ca exchanger current INaCa
      !DPN 7/10/97 - moved to common block
      !kNaCa = 2000.d0   !scaling factor of INaCa (uA/uF)
      !KmNa  = 87.5d0    !half sat.n conc of Na channel (mmol/L)
      KmCa  = LR_KmCa_NaCa !1.38d0    
!half sat.n conc of Ca channel (mmol/L)
      LR_INaCa  = LR_kNaCa*(dexp(LR_eta*V/RTONF)*
     +  LR_Nai**3*LR_Cao
     +  -dexp((LR_eta-1.d0)*V/RTONF)*
     +  LR_Nao**3*LR_Cai)
     +  /((LR_KmNa**3+LR_Nao**3)*(KmCa+LR_Cao)
     +  *(1.d0+LR_ksat*dexp((LR_eta-1.d0)*V/RTONF)))

! Na-K pump INaK
      !IINaK = 1.5d0                   !uA/uF
      !KmNai = 10.d0                   !mmol/L
      !KmKo  = 1.5d0                   !mmol/L
      sigma = (1.d0/7.d0)*(dexp(LR_Nao/67.3d0)-1.d0) 
                                     ![Na]o-dependence factor of fNak
      fNaK  = 1.d0/(1.d0+0.1245d0*dexp(-0.1d0*V/RTONF)
     +        +0.0365d0*sigma*dexp(-V/RTONF))
      LR_INaK = LR_IINaK*fNaK*LR_Ko/
     +  ((LR_Ko+LR_KmKo)*(1.d0+
     +  dsqrt((LR_KmNai/LR_Nai)**2)))

! Non-specific Ca-activated current InsCa
      !KmnsCa = 1.2d-3                 !mmol/L
      !PnsCa  = 1.75d-7                !cm/s
      EnsCa  = RTONF*dlog((LR_Ko+LR_Nao)/
     +  (LR_Ki+LR_Nai))
      Vns    = V-EnsCa
      IInsK  = LR_PnsCa*(Vns/RTONF)*LR_Farad*
     +  (LR_gNai*LR_Nai*dexp(Vns/RTONF)
     +  -LR_gNao*LR_Nao)/(dexp(Vns/RTONF)-1.d0)
      InsK   = IInsK /(1.d0+(LR_KmnsCa/LR_Cai)**3) 
      IInsNa = LR_PnsCa*(Vns/RTONF)*LR_Farad*
     +  (LR_gKi*LR_Ki*dexp(Vns/RTONF)
     +  -LR_gKo*LR_Ko)/(dexp(Vns/RTONF)-1.d0)
      InsNa  = IInsNa/(1.d0+(LR_KmnsCa/LR_Cai)**3)
      LR_InsCa   = InsK + InsNa

! Sarcolemmal Ca pump IpCa
      !KmpCa  = 0.5d-3                 !mmol/L
      !IIpCa  = 1.15d0                 !uA/uF
      LR_IpCa   = LR_IIpCa*(LR_Cai/
     +  (LR_KmpCa+LR_Cai))

! Ca background current ICab
      ECaN   = 0.5d0*RTONF*dlog(LR_Cao/LR_Cai) !Nernst potential of Ca
      !GGCab  = 0.003016d0                !millisiemens/uF
      LR_ICab  = LR_GGCab*(V-ECaN)

! Na background current INab
      ENaN   = ENa                    !Nernst potential of Na
      !GGNab  = 0.00141d0              !millisiemens/uF
      LR_INab  = LR_GGNab*(V-ENaN)

! Total time independent current
      LR_Iv  = LR_IK1+LR_IKp+LR_INaK+
     +  LR_IpCa+LR_ICab+LR_INab


! Compute Cai change at 2 ms after onset of stimulus
      !IF((t.GE.TPS).AND.(t.LT.0.1d0+TPS))THEN
      IF((t.GE.LR_TPS).AND.
     +  (t.LT.LR_TABT+LR_TPS))THEN
        Cai_on = Y(13)
        IF(DEBUG) write (*,*) 'Cai_on = ', Cai_on
      ENDIF

      IF((t.GE.2.d0+LR_TPS).AND.
     +  (t.LT.2.d0+LR_TABT+LR_TPS))THEN
        Cai2 = Y(13)-Cai_on
        IF(DEBUG) write (*,*) 'Cai2 = ', Cai2
      ENDIF

! Ca induced Ca release of JSR
      !Kmrel    = 0.8d-3               !mmol/L
      !Tau_on   = 2.d0                 !ms
      !Tau_off  = 2.d0                 !ms
      !Caith    = 0.18d-3              !mmol/L
      IF(Cai2.GT.LR_Caith) THEN
        GGrel = LR_GGrel_
      ELSE
        GGrel = 0.d0
      ENDIF
      !time of Ca-induced Ca-release
c dpn 03/06/98      t_CICR  = t-TPS
      t_CICR  = t-(LR_TPS+2.0d0)
      Grel  = GGrel*((Cai2-LR_Caith)/
     +  (LR_Kmrel+Cai2-LR_Caith))
     +  *(1.d0-dexp(-t_CICR/LR_Tau_on))*
     +  dexp(-t_CICR/LR_Tau_off)
      LR_Irel = Grel*(LR_CaJSR-LR_Cai)

! Ca release of JSR under Ca-overload conditions
!      CSQNth  = 0.7d0
!      IF(CSQN.GE.CSQNth) THEN
!         GGrel = 4.d0                !ms-1
!      ELSE
!         GGrel = 0.d0                !ms-1
!      ENDIF
!      Grel    = GGrel*(1.d0-dexp(-t/Tau_on))*dexp(-t/Tau_off)
!      I(13)   = Grel*(CaJSR-Cai)


! Ca uptake and leakage of NSR Iup and Ileak
      !Kmup       = 0.92d-3                !mmol/L
      !CaNSRCaNSR = 15.d0                  !mmol/L
      !IIup       = 0.005d0                !mmol/L/ms
      Kleak      = LR_IIup/LR_CaNSRCaNSR        !ms-1
      LR_Ileak   = Kleak*LR_CaNSR            !mmol/L/ms
      LR_Iup     = LR_IIup*(LR_Cai/
     +  (LR_Cai+LR_Kmup)) !mmol/L/ms


! Translocation of Ca ions from NSR to JSR Itr
      !Tau_tr = 180.d0                     !ms
      LR_Itr  = (LR_CaNSR-LR_CaJSR)/
     +  LR_Tau_tr !mmol/L/ms

! Computes current carrying the five ions
c      I_Na  = I(1)+ICaNa+I(6)+I(7)+InsNa+I(11)
      LR_I_Na  = LR_INa+ICaNa+3.0d0*LR_INaCa+
     +  3.0d0*LR_INaK+InsNa+LR_INab
      LR_I_Ca  = ICa-LR_INaCa+LR_IpCa+LR_ICab
c      I_K   = ICaK+I(3)+I(4)+I(5)+I(7)+InsK
      LR_I_K   = ICaK+LR_IK+LR_IK1+LR_IKp-
     +  2.0d0*LR_INaK+InsK
      LR_F_NSR = -LR_Iup+LR_Ileak+LR_Itr
      LR_F_JSR = LR_Irel-LR_Itr*VRatio_JN

      RETURN
      END 


      SUBROUTINE LR_CURRENTS_PLOT(NT_DAT,POTENTIALS,CURRENT,
     '  RESULTS,RPARAM)

C#### Subroutine: LR_CURRENTS_PLOT
C###  Description:
C###    <html><pre>
C###    Calculates the specified current for computing the 
C###    steady state current
C###    CURRENT:- 1 = IICa   - Ca cpt of I(CaL) (maximum)
C###              2 = IICaK  - K cpt of I(CaL)
C###              3 = IICaNa - Na cpt of I(CaL)
C###              4 = IICa + IICaK + IICaNa
C###              6 = IIK
C###              7 = IINa
C###              8 = IIK1
C###    </pre></html>

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/lr001.cmn'
!     Parameter List
      INTEGER NT_DAT,CURRENT
      REAL POTENTIALS(*),RESULTS(*)
      REAL*8 RPARAM(55)
!     Common blocks
      REAL*8        CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
      COMMON /conc/ CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
      REAL*8                   IICa,IICaK,IICaNa,IINa,IIK,IIK1,fca
      COMMON /LR_MAX_CURRENTS/ IICa,IICaK,IICaNa,IINa,IIK,IIK1,fca
!     Local Variables
      INTEGER i
      REAL*8 ALPHA(7),BETA(7),Y(10),t,RTONF
      REAL*8 Currents(19),ICa,ISUMS(5)
      
C *** Initialise dummy arguments
      t = 0.0d0
      DO i=1,7
        ALPHA(i) = 0.0d0
        BETA(i) = 0.0d0
      ENDDO
      DO i=1,10
        Y(i) = 0.0d0
      ENDDO
C *** Initialise required variables
      Ko  = RPARAM(50) !bulk external [K]
      Ki  = RPARAM(51) ![K]i
      Nai = RPARAM(52) ![Na]i
      Nao = RPARAM(55) ![Na]o
      Cai = RPARAM(53) ![Ca]i
      Cao = RPARAM(54) ![Ca]o
      CALL DEFINE_LR(RPARAM) !set parameters
      RTONF = (Temp+273.d0)*86.16d-3 !mV

C *** Calculate the rate coefficients
      DO i=1,NT_DAT
        Y(1)  = DBLE(POTENTIALS(i)) ! V
        Y(2)  = RPARAM(52) ! Nai
        Y(3)  = RPARAM(53) ! Cai
        Y(10) = RPARAM(51) ! Ki
c        CALL LR_CURRENTS(t,Y,ALPHA,BETA,RTONF,Currents,ISUMS,ICa)
        IF (CURRENT.EQ.1) THEN
          RESULTS(i) = SNGL(IICa)
        ELSE IF (CURRENT.EQ.2) THEN
          RESULTS(i) = SNGL(IICaK)
        ELSE IF (CURRENT.EQ.3) THEN
          RESULTS(i) = SNGL(IICaNa)
        ELSE IF (CURRENT.EQ.4) THEN
          RESULTS(i) = SNGL(IICa + IICaK + IICaNa)
        ELSE IF (CURRENT.EQ.7) THEN
          RESULTS(i) = SNGL(IINa)
        ELSE IF (CURRENT.EQ.6) THEN
          RESULTS(i) = SNGL(IIK)
        ELSE IF (CURRENT.EQ.8) THEN
          RESULTS(i) = SNGL(IIK1)
        ENDIF
      ENDDO
      RESULTS(NT_DAT+1) = SNGL(fca) !gating variable
      RETURN
      END


      SUBROUTINE L_R_RATES(Y,ALFA,BETA,RTONF,PARAMETERS,L_PARAMETERS)

C#### Subroutine: L_R_RATES
C###  Description:
C###    <html><pre>
C###    Computes the rate coefficients for the gating variables.
C###    ALFA (1) = m
C###      "   2  = h
C###      "   3  = j
C###      "   4  = d
C###      "   5  = f
C###      "   6  = X
C###      "   7  = K1 
C###    BETA (1) = m
C###      "   2  = h
C###      "   3  = j
C###      "   4  = d
C###      "   5  = f
C###      "   6  = X
C###      "   7  = K1
C###    </pre></html>

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
c      INCLUDE 'cell/deoxs00.cmn'
c      INCLUDE 'cell/lr001.cmn'
!     Parameter List
      INTEGER L_PARAMETERS
      REAL*8 ALFA(*),BETA(*),Y(*),RTONF,PARAMETERS(L_PARAMETERS)
!     Common blocks
c      REAL*8        CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
c      COMMON /conc/ CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
!     Local Variables
      REAL*8 d0,f0,Tau_d,Tau_f
      REAL*8 V,EK1,Ki ! SMAR009 removed ->Ko !,RTONF,TEMP,Ko,Ki
      
      V = Y(1)
      !Temp  = 37.d0                   !temperature (deg C)
      !RTONF = (Temp+273.d0)*86.16d-3  !RT/F

! Standard ionic concentrations
      Ki   = Y(10)                    ![K]i
      !Ko   = 5.4d0                    ![K]o

! Compute the K1 reversal potential (mV)
      EK1 = RTONF*dlog(LR_Ko/Ki)
      !K1 reversal potential (mV)

! Fast Na channel gates m,h,j
      ALFA(1) = 0.32d0*(V+47.13d0)/(1.d0-dexp(-0.1d0*(V+47.13d0)))   !m
      BETA(1) = 0.08d0*dexp(-V/11.d0) !m
      IF(V.GE.-40.d0) THEN       
        ALFA(2) = 0.d0 !h
        BETA(2) = 1.d0/(0.13d0*(1.d0+dexp((V+10.66d0)/(-11.1d0))))!h
        ALFA(3) = 0.d0 !j
        BETA(3) = 0.3d0*dexp(-2.535d-7*V)/(1.d0+dexp(-0.1d0*(V+32.d0)))
      ELSE IF(V.LT.-40.d0) THEN
        ALFA(2) = 0.135d0*dexp((80.d0+V)/(-6.8d0))!h
        BETA(2) = 3.56d0*dexp(0.079d0*V)+(3.1d5*dexp(0.35d0*V))!h
        ALFA(3) = (-1.2714d5*dexp(0.2444d0*V)-3.474d-5*
     +            dexp(-0.04391d0*V))*(V+37.78d0)/(1.d0+dexp(0.311d0*
     +            (V+79.23d0)))  !j
        BETA(3) = 0.1212d0*dexp(-0.01052d0*V)/(1.d0+dexp(-0.1378d0
     +            *(V+40.14d0))) !j
      ENDIF

! L-type Ca channel gates d,f
      d0    = 1.d0/(1.d0+dexp(-(V+10.d0)/6.24d0))
      Tau_d = d0*(1.d0-dexp(-(V+10.d0)/6.24d0))/(0.035d0*(V+10.d0))
      f0    = (1.d0/(1.d0+dexp((V+35.06d0)/8.6d0)))+(0.6d0/
     +        (1.d0+dexp((50.d0-V)/20.d0)))
      Tau_f = 1.d0/(0.0197d0*dexp(-((0.0337d0*(V+10.d0))**2))+0.02d0)

      ALFA(4) = d0/Tau_d              !d
      BETA(4) = (1.d0-d0)/Tau_d       !d
      ALFA(5) = f0/Tau_f              !f
      BETA(5) = (1.d0-f0)/Tau_f       !f

! Time-dep. K channel gate X
      ALFA(6) = 7.19d-5*(V+30.d0)/(1.d0-dexp(-0.148d0*(V+30.d0)))  ! X
      BETA(6) = 1.31d-4*(V+30.d0)/(-1.d0+dexp(0.0687d0*(V+30.d0))) ! X

! Time-indep. K channel gate K1
      ALFA(7) = 1.02d0/(1.d0+dexp(0.2385d0*(V-EK1-59.215d0)))
      BETA(7) = (0.49124d0*dexp(0.08032d0*(V-EK1+5.476d0))
     +  +dexp(0.06175d0*
     +  (V-EK1-594.31d0)))
     +  /(1.d0+dexp(-0.5143d0*
     +  (V-EK1+4.753d0)))

      RETURN
      END 


      SUBROUTINE LR_RATES_PLOT(KI_VALUES,NT_DAT,POTENTIALS,RATE,
     '  RESULTS,RPARAM)

C#### Subroutine: LR_RATES_PLOT
C###  Description:
C###    Calculates the specified reaction rate coefficient pair 
C###    (alpha/beta)

      IMPLICIT NONE
c      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/lr001.cmn'
!     Parameter List
      INTEGER NT_DAT,RATE
      REAL KI_VALUES(*),POTENTIALS(*),RESULTS(*),RPARAM(3)
!     Common blocks
      REAL*8        CaNSR,CaJSR,Ko
      COMMON /conc/ CaNSR,CaJSR,Ko
!     Local Variables
      INTEGER i
      REAL*8 ALPHA(7),BETA(7),Y(10),RPARAM1(50),RTONF
      
C *** Initialise required variables
      Ko   = DBLE(RPARAM(1))
      RPARAM1(16) = DBLE(RPARAM(2)) !temp
      CALL DEFINE_LR(RPARAM1)       !set RTONF
      RTONF = (Temp+273.d0)*86.16d-3 !mV
C *** Calculate the rate coefficients
      DO i=1,NT_DAT
        Y(1)  = DBLE(POTENTIALS(i))
        Y(10) = DBLE(KI_VALUES(i))
c        CALL L_R_RATES(Y,ALPHA,BETA,RTONF)
        RESULTS(i)        = SNGL(ALPHA(RATE)) !get the reaction
        RESULTS(i+NT_DAT) = SNGL(BETA(RATE))  !rate coefficients
      ENDDO

      RETURN
      END


c      SUBROUTINE LUO_RUDY(IPARAM_1,IPARAM_2,LPARAM_2,RPARAM,
c     '  RPARAM_2,RPARAM_LR,NT_DAT,Y_FULL)

      SUBROUTINE LUO_RUDY(AII,AIO,CONTROL,ERROR_TYPE,IFAIL,IWORK,
     '  L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_IWORK,L_MODEL,
     '  L_PARAMETERS,L_WORK,MAX_ITERS,MAX_ORDER,MODEL,
     '  ABS_ERR,ARI,ARO,MAX_STEP,PARAMETERS,REL_ERR,WORK,Y_FULL,
     '  EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL,ERROR_C)

C#### Subroutine: LUO_RUDY
C###  Description:
C###    Solve Luo-Rudy equations.
C###    Voltage is in mV & time in ms.

      IMPLICIT NONE

! Passed variables
      INTEGER L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_IWORK,L_MODEL,
     '  L_PARAMETERS,L_WORK
      INTEGER AII(L_AII),AIO(L_AIO),CONTROL(L_CONTROL),ERROR_TYPE,
     '  IFAIL,IWORK(L_IWORK),MAX_ITERS,MAX_ORDER,MODEL(L_MODEL)
      REAL*8 ABS_ERR,ARI(L_ARI),ARO(L_ARO),MAX_STEP,
     '  PARAMETERS(L_PARAMETERS),REL_ERR,WORK(L_WORK)
      REAL*4 Y_FULL(*)
      LOGICAL EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL
      INTEGER ERROR_C(*)
!     Common blocks
      REAL*8        I_Na,I_Ca,I_K,F_NSR,F_JSR
      COMMON /isum/ I_Na,I_Ca,I_K,F_NSR,F_JSR
cp      REAL*8        TRPN,CMDN,CSQN
cp      COMMON /buff/ TRPN,CMDN,CSQN
      REAL*8        I(19),ICa !DPN 3/10/97 - extending I()
      COMMON /curr/ I,ICa
C      REAL*8        TIMESCALE,TSTART,TEND,DT,TABT,TPS,TP
C      COMMON /times/ TIMESCALE,TSTART,TEND,DT,TABT,TPS,TP
      REAL*8        CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
      COMMON /conc/ CaNSR,CaJSR,Ko !,Nai,Ki,Cai,Nao,Cao
      REAL*8        Cai_on,Cai2
      COMMON /Cai_/ Cai_on,Cai2
! Local variables
      INTEGER NO_CUR,NO_DAT,NO_EQN,NO_FULL,NT_EQN,NT_CUR,
     ' IFLAG,NT_DAT,NT_ADD,NT_MECH,NO_ADD
      REAL*8 RPARAM(21,9),RPARAM_2(9),RPARAM_LR(50)
!     SMAR009 23/12/98 ,J,NO_CON,error,
      REAL*8 TCURRENT,TNEXT,Y_EQN(50),DY(50) !,CON(20),CUR(20)
      CHARACTER ERROR_FORTRAN*(200)

      external LR

c      IF(DEBUG) write(*,'('' >>>call LUO_RUDY'')')

C DPN 3/11/97 - initialise Cai_on and Cai2 each time this function is
C               called. Required due to static variables ???
      Cai_on = 0.0d0
      Cai2 = 0.0d0


      NT_DAT  = AII(1)
      NT_EQN  = AII(2)
      NT_ADD  = AII(3)




      !set initial values
      LR_Number_of_stimulii = 1
      NO_FULL = 0
      DO NO_EQN=1,NT_EQN
        NO_FULL=NO_FULL+1
        Y_EQN(NO_EQN)=DBLE(Y_FULL(NO_EQN))
      ENDDO
      IF (NT_ADD.GT.0) THEN
        DO NO_ADD=1,NT_ADD
          NO_FULL=NO_FULL+1
          Y_FULL(NO_FULL)=0.0e0
        ENDDO
      ENDIF

C *** Calculate the solution.  Print and save after each TABT
      TCURRENT=LR_TSTART
      IFAIL=1 !for 1st call to ADAMS_MOULTON
      NO_DAT=1
      DO WHILE ((IFAIL.LE.2).AND.(NO_DAT.LT.NT_DAT))
        TNEXT=TCURRENT+LR_TABT
        CALL ADAMS_MOULTON(AII,AIO,CONTROL,ERROR_TYPE,IFAIL,IWORK,
     '    L_AII,L_AIO,L_ARI,L_ARO,L_CONTROL,L_IWORK,L_MODEL,
     '    L_PARAMETERS,L_WORK,MAX_ITERS,MAX_ORDER,MODEL,NT_EQN,
     '    ABS_ERR,ARI,ARO,DY,MAX_STEP,PARAMETERS,REL_ERR,TCURRENT,
     '    TNEXT,WORK,Y_EQN,EXTEND_INTERVAL,STIFF_EQNS,USE_ROUND_CTRL,
     '    LR,ERROR_FORTRAN)
        IF(IFAIL.EQ.2) THEN
          DO NO_EQN=1,NT_EQN
            NO_FULL=NO_FULL+1
            Y_FULL(NO_FULL)=SNGL(Y_EQN(NO_EQN))
          ENDDO
          IF (NT_ADD.GT.0) THEN
            DO NO_ADD=1,NT_ADD
              NO_FULL=NO_FULL+1
              Y_FULL(NO_FULL)=SNGL(ARO(NO_ADD))
            ENDDO
          ENDIF
          TCURRENT=TNEXT
          NO_DAT=NO_DAT+1
        ENDIF !iflag
      ENDDO !while iflag
      
C      close(unit=1,status='keep')
C      close(unit=99,status='keep')

      RETURN
      END 


      SUBROUTINE NOBLE98(t,Y,F)

C#### Subroutine: NOBLE98
C###  Description:
C###    Calculates the RHS of the Noble '98 set of ODE's

      IMPLICIT NONE
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs005.cmn'
      !passed variables
      REAL*8 t,Y(*),F(*)
      !common blocks
      REAL*8 INa98,IpNa98,IbNa98,IK198,IKr98,IKs98,IbK98,IKATP98,
     '  IKNa98,IK98,ICaLCa98,ICaLK98,ICaLNa98,ICaLCads98,ICaLKds98,
     '  ICaLNads98,IbCa98,IKACH98,INaK98,INaCa98,INaCads98,Iup98,
     '  Itr98,Irel98,XSRrel98,Ito98
      COMMON /OH5_CURRENTS/
     '  INa98,IpNa98,IbNa98,IK198,IKr98,IKs98,IbK98,IKATP98,
     '  IKNa98,IK98,ICaLCa98,ICaLK98,ICaLNa98,ICaLCads98,ICaLKds98,
     '  ICaLNads98,IbCa98,IKACH98,INaK98,INaCa98,INaCads98,Iup98,
     '  Itr98,Irel98,XSRrel98,Ito98
      REAL*8 OH5_Vcell,OH5_Vi,OH5_Vds,OH5_VSRup
      COMMON /OH5_VOLUMES/
     '  OH5_Vcell,OH5_Vi,OH5_Vds,OH5_VSRup
      REAL*8 OH5_Istim
      COMMON /OH5_STIMULUS/
     '  OH5_Istim
      REAL*8    OH5_kcachoff,OH5_kmk1,OH5_kmK,
     '          OH5_kmNa,OH5_kNaCa,OH5_kmCa,
     '          OH5_k1,OH5_k2,OH5_k3,OH5_k4,
     '          OH5_kATP,OH5_KD,OH5_kkNa,
     '          OH5_kdsoff,OH5_kmCads,OH5_kdecay,
     '          OH5_Rdecay,OH5_INaKmax
      COMMON /OH5_PARAM1/
     '  OH5_kcachoff,OH5_kmk1,OH5_kmK,
     '  OH5_kmNa,OH5_kNaCa,OH5_kmCa,
     '  OH5_k1,OH5_k2,OH5_k3,OH5_k4,
     '  OH5_kATP,OH5_KD,OH5_kkNa,
     '  OH5_kdsoff,OH5_kmCads,OH5_kdecay,
     '  OH5_Rdecay,OH5_INaKmax
      REAL*8    OH5_GNa,OH5_GbK,OH5_GK1,OH5_GbNa,
     '          OH5_GpNa,OH5_GbCa,
     '          OH5_GKr1,OH5_GKr2,OH5_GKs,OH5_GKATP,OH5_GKACh,
     '          OH5_PCa,OH5_PCaK,OH5_PCaNa,OH5_PKNa
      COMMON /OH5_PARAM2/
     '  OH5_GNa,OH5_GbK,OH5_GK1,OH5_GbNa,
     '  OH5_GpNa,OH5_GbCa,
     '  OH5_GKr1,OH5_GKr2,OH5_GKs,OH5_GKATP,OH5_GKACh,
     '  OH5_PCa,OH5_PCaK,OH5_PCaNa,OH5_PKNa
      REAL*8    OH5_Cab,OH5_Ko,OH5_Nao,
     '          OH5_FractICaL,OH5_FractINaCa,
     '          OH5_DIFFCa,OH5_dNaCa,OH5_gama,OH5_nNaCa,
     '          OH5_kcyCa,OH5_kxcs,OH5_ksrCa,OH5_Cm,
     '          OH5_ALPHA12,OH5_BETA12
      COMMON /OH5_PARAM3/
     '  OH5_Cab,OH5_Ko,OH5_Nao,
     '  OH5_FractICaL,OH5_FractINaCa,
     '  OH5_DIFFCa,OH5_dNaCa,OH5_gama,OH5_nNaCa,
     '  OH5_kcyCa,OH5_kxcs,OH5_ksrCa,OH5_Cm,
     '  OH5_ALPHA12,OH5_BETA12
      REAL*8    OH5_F,OH5_R,OH5_T,
     '          OH5_Vecs,OH5_radius,OH5_length,
     '          OH5_Vup,OH5_Vrel,OH5_kmCa2,
     '          OH5_Mtrop,OH5_Ctrop,OH5_alfatrop,
     '          OH5_betatrop,OH5_gamatropSL,
     '          OH5_gamaSRSL,OH5_sacSL
      COMMON /OH5_PARAM4/
     '  OH5_F,OH5_R,OH5_T,
     '  OH5_Vecs,OH5_radius,OH5_length,
     '  OH5_Vup,OH5_Vrel,OH5_kmCa2,
     '  OH5_Mtrop,OH5_Ctrop,OH5_alfatrop,
     '  OH5_betatrop,OH5_gamatropSL,
     '  OH5_gamaSRSL,OH5_sacSL
      REAL*8    OH5_G_Ca_stretch,OH5_G_Na_stretch,OH5_G_K_stretch,
     '          OH5_G_Ns_stretch,OH5_G_An_stretch,
     '          OH5_E_Ns_stretch,OH5_E_An_stretch
      COMMON /OH5_PARAM5/
     '  OH5_G_Ca_stretch,OH5_G_Na_stretch,OH5_G_K_stretch,
     '  OH5_G_Ns_stretch,OH5_G_An_stretch,
     '  OH5_E_Ns_stretch,OH5_E_An_stretch
      REAL*8    OH5_alfaS,OH5_alfaV,
     '          OH5_alfaACh,OH5_betaACh,
     '          OH5_SLhst,OH5_IThst
      COMMON /OH5_PARAM6/
     '  OH5_alfaS,OH5_alfaV,
     '  OH5_alfaACh,OH5_betaACh,
     '  OH5_SLhst,OH5_IThst
      REAL*8 OH5_kdsdecay,OH5_Jsrleak
      COMMON /OH5_PARAM7/
     '  OH5_kdsdecay,OH5_Jsrleak
      !local variables
      REAL*8 V,mNa,hNa,dCa,fCa,f2Ca,xr1,xr2,xs,xACH,Cads,
     '  CaCALMOD,CaTROP,factivator,fproduct,rto,qto !SMAR009 21/12/98 Caup,Carel,
C *** stored in deoxs00.cmn
      REAL*8 OH5_Nai,OH5_Ki,OH5_Cao,OH5_Cai
      REAL*8 RTF,ENa,EK,EKs,ECa,Emh,POTS(5)
      REAL*8 alfa_mNa,beta_mNa,alfa_hNa,beta_hNa,alfa_dCa,beta_dCa,
     '  alfa_fCa,beta_fCa,alfa_xr1,beta_xr1,alfa_xr2,beta_xr2,
     '  alfa_xs,beta_xs,alfa_xACH,beta_xACH,alfa_rto,beta_rto
      REAL*8 ALPHA(9),BETA(9)
      REAL*8 INa,IpNa,IbNa,IK1,IbK,IKATP, !SMAR009 21/12/98 ,IKr,IKs,
     '  IKNa,IK,ICaLCa,ICaLK,ICaLNa,ICaLCads,ICaLKds,
     '  ICaLNads,IbCa,IKACH,INaK,INaCa,INaCads,Iup,
     '  Itr,Irel,XSRrel,Ito,I(26)

      RTF = OH5_R*OH5_T/OH5_F

! State variables
C ** DPN 04/11/98 - add f2Ca, slow ICaL inactivation
C ** DPN 05/11/98 - add Ito - r & q variables
      V          = y( 1) !membrane potential
      mNa        = y( 7) !Na channel m gate
      hNa        = y( 6) !Na channel h gate
      dCa        = y( 4) !Ca channel d gate
      fCa        = y( 5) !Ca channel f gate
      f2Ca       = y(11) !Ca channel slow inactivation gate
      xr1        = y(20) !fast K channel, fast rate
      xr2        = y(21) !fast K channel, slow rate
      xs         = y(22) !slow K channel
      xACh       = y(23) !??
      rto        = y(12) !inactivation variable for ito
      qto        = y(13) !activation variable for ito
      Oh5_Nai        = y( 2) !intracellular [Na] 
      Oh5_Ki         = y( 8) !intracellular [K]
      Oh5_Cao        = y(16) !extracellular [Ca]
      Oh5_Cai        = y( 3) !intracellular [Ca]
      Cads       = y(19) !sub-space [Ca]
c      Caup       = y( 9) ![Ca]NSR
c      Carel      = y(10) ![Ca]JSR
      CaCALMOD   = y(14) ![Ca] bound to calmodulin
      CaTROP     = y(15) ![Ca] bound to troponin
      factivator = y(17) !??? Ca SR release rate ???
      fproduct   = y(18) !???

! Equilibrium potentials
c      ENa  = RTF*DLOG(OH5_Nao/Oh5_Nai)
                        !Na channel
      IF(OH5_Nao/Oh5_Nai.GT.0.0d0) THEN
        ENa  = RTF*DLOG(OH5_Nao/Oh5_Nai)                 !Na channel
        POTS(1) = ENa
      ELSE
        WRITE(*,*) ' >>Attempted to log a -ve (Na)'
      ENDIF
      IF(OH5_Ko/Oh5_Ki.GT.0.0d0) THEN
        EK   = RTF*DLOG(OH5_Ko/Oh5_Ki)
        POTS(2) = EK
      ELSE
        WRITE(*,*) ' >>Attempted to log a -ve (K)'
      ENDIF 
                         !K  channel
c      EKs  = RTF*DLOG((OH5_Ko+OH5_PkNa*OH5_Nao)/(Oh5_Ki+OH5_PkNa*Oh5_Nai)) 
      IF((OH5_Ko+OH5_PkNa*OH5_Nao)/(Oh5_Ki+Oh5_Nai).GT.0.0d0) THEN
        EKs  = RTF*DLOG((OH5_Ko+OH5_PkNa*OH5_Nao)/(Oh5_Ki+Oh5_Nai)) 
        POTS(3) = EKs
      ELSE
        WRITE(*,*) ' >>Attempted to log a -ve (K2)'
      ENDIF

!Ks channel
c      ECa  = RTF*DLOG(Oh5_Cao/Oh5_Cai)                           !Ca
      IF(Oh5_Cai/Oh5_Cao.GT.0.0d0) THEN
        ECa  = -0.5d0*RTF*DLOG(Oh5_Cai/Oh5_Cao)                     !Ca
        POTS(4) = ECa
      ELSE
        WRITE(*,*) ' >>Attempted to log a -ve (Ca)'
        WRITE(*,'(''   [Ca]i = '',F12.6,'' [Ca]o = '',F12.6)') Oh5_Cai,
     '    Oh5_Cao
      ENDIF
      IF((OH5_Nao+0.12d0*OH5_Ko)/(Oh5_Nai+0.12d0*Oh5_Ki).GT.0.0d0) THEN
        Emh  = RTF*DLOG((OH5_Nao+0.12d0*OH5_Ko)/(Oh5_Nai+0.12d0*Oh5_Ki))
        POTS(5) = Emh
      ELSE
        WRITE(*,*) ' >>Attempted to log a -ve (NaK)'
      ENDIF

      CALL NOBLE98_CHANGE(t)

      CALL NOBLE98_RATES(V,Oh5_Cai,ALPHA,BETA)
      alfa_mNa = ALPHA(1)
      beta_mNa = BETA(1)
      alfa_hNa = ALPHA(2)
      beta_hNa = BETA(2)
      alfa_dCa = ALPHA(3)
      beta_dCa = BETA(3)
      alfa_fCa = ALPHA(4)
      beta_fCa = BETA(4)
      alfa_xr1 = ALPHA(5)
      beta_xr1 = BETA(5)
      alfa_xr2 = ALPHA(6)
      beta_xr2 = BETA(6)
      alfa_xs = ALPHA(7)
      beta_xs = BETA(7)
      alfa_xACH = ALPHA(8)
      beta_xACH = BETA(8)
      alfa_rto = ALPHA(9)
      beta_rto = BETA(9)

      CALL NOBLE98_CURRENTS(Y,RTF,POTS,I)  ! calculate ionic currents
      INa = I(1)
      IpNa = I(2)
      IbNa = I(3)
      IK1 = I(4)
c      IKr = I(5)
c      IKs = I(6)
      IbK = I(7)
      IKATP = I(8)
      IKNa = I(9)
      IK = I(10)
      ICaLCa = I(11)
      ICaLK = I(12)
      ICaLNa = I(13)
      ICaLCads = I(14)
      ICaLKds = I(15)
      ICaLNads = I(16)
      IbCa = I(17)
      IKACH = I(18)
      INaK = I(19)
      INaCa = I(20)
      INaCads = I(21)
      Iup = I(22)
      Itr = I(23)
      Irel = I(24)
      XSRrel = I(25)
      Ito = I(26)

      IF (SINGLE_CELL) THEN
        !need this to get currents back to UNEMAP for plotting
        !"fast fix, temp hack" - Chris Bradley, 02 December 1998 !!!!!
        INa98 = I(1)
        IpNa98 = I(2)
        IbNa98 = I(3)
        IK198 = I(4)
        IKr98 = I(5)
        IKs98 = I(6)
        IbK98 = I(7)
        IKATP98 = I(8)
        IKNa98 = I(9)
        IK98 = I(10)
        ICaLCa98 = I(11)
        ICaLK98 = I(12)
        ICaLNa98 = I(13)
        ICaLCads98 = I(14)
        ICaLKds98 = I(15)
        ICaLNads98 = I(16)
        IbCa98 = I(17)
        IKACH98 = I(18)
        INaK98 = I(19)
        INaCa98 = I(20)
        INaCads98 = I(21)
        Iup98 = I(22)
        Itr98 = I(23)
        Irel98 = I(24)
        XSRrel98 = I(25)
        Ito98 = I(26)
      ENDIF

C      ISWTCH(1) = 0.0d0 !INa
C      ISWTCH(2) = 1.0d0 !IbNa
C      ISWTCH(3) = 1.0d0 !IpNa
C      ISWTCH(4) = 1.0d0 !INaK
C      ISWTCH(5) = 0.0d0 !INaCa
C      ISWTCH(6) = 1.0d0 !INaCads
C      ISWTCH(7) = 1.0d0 !IK1
C      ISWTCH(8) = 1.0d0 !IK
C      ISWTCH(9) = 1.0d0 !IKATP
C      ISWTCH(10) = 1.0d0 !IKACh
C      ISWTCH(11) = 1.0d0 !IbK
C      ISWTCH(12) = 1.0d0 !ICaLNa
C      ISWTCH(13) = 1.0d0 !ICaLK
C      ISWTCH(14) = 1.0d0 !ICaLCa
C      ISWTCH(15) = 1.0d0 !ICaLNads
C      ISWTCH(16) = 1.0d0 !ICaLKds
C      ISWTCH(17) = 1.0d0 !ICaLCads
C      ISWTCH(18) = 1.0d0 !IbCa

!d(V)/dt     
      F( 1) = -(ISWTCH(16)*INa+ISWTCH(4)*IbNa+ISWTCH(23)*IpNa+
     '  ISWTCH(18)*INaK+ISWTCH(17)*INaCa+ISWTCH(19)*INaCads !eqn 1
     '  +ISWTCH(14)*IK1+ISWTCH(13)*IK+ISWTCH(15)*IKATP+
     '  ISWTCH(1)*IKACh+ISWTCH(3)*IbK 
     '  +ISWTCH(10)*ICaLNa+ISWTCH(9)*ICaLK+ISWTCH(8)*ICaLCa+
     '  ISWTCH(7)*ICaLNads+ISWTCH(6)*ICaLKds+ISWTCH(5)*ICaLCads
     '  +ISWTCH(2)*IbCa+ISWTCH(20)*Ito+ISWTCH(21)*IKNa+
     '  OH5_Istim) / OH5_Cm
c      F( 1) = -(INa+IbNa+IpNa+INaK+INaCa+INaCads                !eqn 1
c     '  +IK1+IK+IKATP+IKACh+IbK 
c     '  +ICaLNa+ICaLK+ICaLCa+ICaLNads+ICaLKds+ICaLCads
c     '  +IbCa+OH5_Istim) / OH5_Cm
!d(mNa)/dt                                                          
      F( 7) = alfa_mNa *(1.d0-mNa ) - beta_mNa *mNa            !eqn 2
!d(hNa)/dt                                                         
      F( 6) = alfa_hNa *(1.d0-hNa ) - beta_hNa *hNa            !eqn 3 
!d(dCa)/dt                                                          
      F( 4) = alfa_dCa *(1.d0-dCa ) - beta_dCa *dCa            !eqn 4
!d(fCa)/dt                                                          
      F( 5) = alfa_fCa *(1.d0-fCa ) - beta_fCa *fCa            !eqn 5
C ** DPN 04/11/98 - add f2Ca, slow ICaL inactivation
!d(f2Ca)/dt - slow ICaL inactivation - from PASCAL code
      F(11) = OH5_Rdecay*(1.0d0-Cads/(Cads+OH5_Kdsoff)-f2Ca)
!d(xr1)/dt                                                          
      F(20) = alfa_xr1 *(1.d0-xr1 ) - beta_xr1 *xr1            !eqn 7
!d(xr2)/dt                                                          
      F(21) = alfa_xr2 *(1.d0-xr2 ) - beta_xr2 *xr2            !eqn 8
!d(xs)/dt
      F(22) = alfa_xs  *(1.d0-xs  ) - beta_xs  *xs             !eqn 9
!d(xACh)/dt
      F(23) = alfa_xACh*(1.d0-xACh) - beta_xACh*xACh           !eqn 10
C ** DPN 05/11/98 - add Ito - r & q variables
!d(rto)/dt
      F(12) = alfa_rto*(1.0d0-rto) - beta_rto*rto
c      F(12) = 0.0d0
!d(qto)/dt SPEED[19] = 1.0 ???????
      F(13) = 333.0d0*(1.0d0/(1.0d0+DEXP(-(V+4.0d0)/5.0d0))-qto)
c      F(13) = 0.0d0
!d(Nai)/dt
C ** DPN 04/11/98 - new from HEART code
C      F( 2) = -(INa+IpNa+IbNa+3.d0*(INaK+INaCa+      !eqn 11
C     '  INaCads)+ICaLNa)/(OH5_Vi*OH5_F)
      F( 2) = -(INa+IpNa+(IbNa*OH5_Nao/140.0d0)+
     '  3.d0*(INaK+INaCa)+ICaLNa)/(OH5_Vi*OH5_F)
!d(Ki)/dt
C ** DPN 04/11/98 - new from HEART code
C      F( 8) = -(IK1+ICaLK+IbK+IK-2.d0*INaK)          !eqn 12
C     '        /(OH5_Vi*OH5_F)
      F( 8) = -(IK1+ICaLK+IbK+IK-2.d0*INaK+ICaLKds
     '  +IKATP+IKNa+Ito)/(OH5_Vi*OH5_F)
!d(Cao)/dt
c      F(16) =  (ICaLCa+ICaLCads+IbCa-2.d0*INaCa)       !eqn 13
c     '        /(2.d0*OH5_Vcell*OH5_Vecs*OH5_F)
c     '        - OH5_DIFFCa*(Oh5_Cao-OH5_Cab)         
c      F(16) =  (ICaLCa+IbCa-2.d0*INaCa)       !eqn 13
c     '        /(2.d0*OH5_Vcell*OH5_Vecs*OH5_F)
c     '        - OH5_DIFFCa*(Oh5_Cao-OH5_Cab)         
      F(16) = 0.0d0
C ** DPN 05/11/98 - need F(14) and F(15) before can calculate F(3)
!d(CaCALMOD)/dt
      F(14) = 1.d5*(OH5_Mtrop-CaCALMOD)*Oh5_Cai - 50.d0*CaCALMOD   !eqn 18
!d(CaTROP)/dt
C ** DPN 04/11/98 - new from HEART code
C      F(13) = OH5_alfatrop*DEXP(OH5_gamatropSL)*Oh5_Cai*(OH5_Ctrop-CaTROP)
C     '  -OH5_betatrop*CaTROP                                   !eqn 19
      F(15) = OH5_alfatrop*Oh5_Cai*(OH5_Ctrop-CaTROP)
     '  -OH5_betatrop*CaTROP                                   !eqn 19
!d(Cai)/dt   
C ** DPN 04/11/98 - new from HEART code
C      F( 3) =  -(ICaLCa+IbCa-2.d0*INaCa)/(2.d0*OH5_Vi*OH5_F) !eqn 14
C     '        - Iup + Irel*OH5_VSRup*OH5_Vrel/(OH5_Vi*OH5_Vup)
C     '        - CaCALMOD - CaTROP 
C     '        + Cads/(Cads+OH5_kdecay)*OH5_Rdecay
c      F( 3) =  -(ICaLCa+IbCa-2.d0*INaCa)/(2.d0*OH5_Vi*OH5_F) !eqn 14
c     '        - Iup + Irel*OH5_VSRup*OH5_Vrel/(OH5_Vi*OH5_Vup)
c     '        - F(12) - F(13) + Cads*OH5_Kdsdecay*OH5_Vds
      F( 3) =  -(ICaLCa+IbCa-2.d0*INaCa)/(2.d0*OH5_Vi*OH5_F) !eqn 14
     '        - Iup + Irel*OH5_VSRup*OH5_Vrel/(OH5_Vi*OH5_Vup)
     '        - F(14) - F(15) + Cads*OH5_Kdsdecay*OH5_Vds/OH5_Vi
!d(Cads)/dt
C ** DPN 04/11/98 - new from HEART code
C      F(17) = -(ICaLCads-2.d0*INaCads)                     !eqn 15
C     '        /(2.d0*OH5_Vds*OH5_F)
C     '        - Cads/(Cads+OH5_kdecay)*OH5_Rdecay
c      F(17) = -(ICaLCads)/(2.d0*OH5_Vds*OH5_Vi*OH5_F)
c     '        - Cads*OH5_Kdsdecay
      F(19) = -(ICaLCads)/(2.d0*OH5_Vds*OH5_F)
     '        - Cads*OH5_Kdsdecay
!d(Caup)/dt
      F( 9) = OH5_Vi/OH5_VSRup*Iup - Itr                   !eqn 16
!d(Carel)/dt
      F(10) = OH5_Vup/OH5_Vrel*Itr - Irel                  !eqn 17
!d(factivator)dt
C ** DPN 04/11/98 - new from HEART code
C      F(15) = (1.0d0-factivator-fproduct)*(5.d2*XSRrel**2+6.d2* !eqn20
C     '  DEXP((V-4.d1)*8.d-2))-factivator*(5.d2*XSRrel**2+6.d1)
      F(17) = (1.0d0-factivator-fproduct)*(5.d2*XSRrel**2+0.0d0*
     '  DEXP((V-4.d1)*8.d-2))-factivator*(5.d2*XSRrel**2+6.d1)
!d(fproduct)/dt
      F(18) = factivator*(5.d2*XSRrel**2+6.d1)-fproduct      !eqn 21
      IF (V.LT.-50.0d0) THEN
        F(17) = F(17) * 5.0d0
        F(18) = F(18) * 5.0d0
      ENDIF
      RETURN
      END


      SUBROUTINE NOBLE98_CHANGE(TIME)

C#### Subroutine: NOBLE98_CHANGE
C###  Description:
C###    Sets the stimulation current for the Noble '98 model based on
C###    the current time.
C###
C###    ??? DPN - needs upgrading to include different pacing schemes

      IMPLICIT NONE
      INCLUDE 'cell/deoxs00.cmn'
      !passed variables
      REAL*8 TIME
      !common blocks
      REAL*8 OH5_Istim
      COMMON /OH5_STIMULUS/
     '  OH5_Istim
      REAL*8 OH5_STIMSIZE,OH5_FREQ
      COMMON /OH5_STIM/ 
     '  OH5_STIMSIZE,OH5_FREQ
      !local variables
      REAL*8 PERIOD !SMAR009 23/12/98 ,stimulation_time

C *** Set the stimulus curent
      IF (time.GE.TPS.AND.time.LE.TPS+TP) THEN
        OH5_Istim = OH5_STIMSIZE
      ELSE
        OH5_Istim = 0.0d0
      ENDIF

C *** Evaluate the new stimulus time
      IF(OH5_FREQ.GT.0.0d0) THEN
        IF (time.GT.TPS+TP) THEN
          PERIOD = 1.0d0 / OH5_FREQ !freq in Hz
          TPS = TPS + PERIOD
        ENDIF
      ENDIF

      RETURN
      END


      SUBROUTINE NOBLE98_CURRENTS(Y,RTF,POTS,I)

C#### Subroutine: NOBLE98_CURRENTS
C###  Description:
C###    Calculates the ionic currents for the Noble '98 model.

      IMPLICIT NONE
!     SMAR009  INCLUDE 'cell/cell00.cmn'
      !passed variables
      REAL*8 Y(*),RTF,POTS(5),I(26)
      !common blocks
      REAL*8    OH5_kcachoff,OH5_kmk1,OH5_kmK,
     '          OH5_kmNa,OH5_kNaCa,OH5_kmCa,
     '          OH5_k1,OH5_k2,OH5_k3,OH5_k4,
     '          OH5_kATP,OH5_KD,OH5_kkNa,
     '          OH5_kdsoff,OH5_kmCads,OH5_kdecay,
     '          OH5_Rdecay,OH5_INaKmax
      COMMON /OH5_PARAM1/
     '  OH5_kcachoff,OH5_kmk1,OH5_kmK,
     '  OH5_kmNa,OH5_kNaCa,OH5_kmCa,
     '  OH5_k1,OH5_k2,OH5_k3,OH5_k4,
     '  OH5_kATP,OH5_KD,OH5_kkNa,
     '  OH5_kdsoff,OH5_kmCads,OH5_kdecay,
     '  OH5_Rdecay,OH5_INaKmax
      REAL*8    OH5_GNa,OH5_GbK,OH5_GK1,OH5_GbNa,
     '          OH5_GpNa,OH5_GbCa,
     '          OH5_GKr1,OH5_GKr2,OH5_GKs,OH5_GKATP,OH5_GKACh,
     '          OH5_PCa,OH5_PCaK,OH5_PCaNa,OH5_PKNa
      COMMON /OH5_PARAM2/
     '  OH5_GNa,OH5_GbK,OH5_GK1,OH5_GbNa,
     '  OH5_GpNa,OH5_GbCa,
     '  OH5_GKr1,OH5_GKr2,OH5_GKs,OH5_GKATP,OH5_GKACh,
     '  OH5_PCa,OH5_PCaK,OH5_PCaNa,OH5_PKNa
      REAL*8    OH5_Cab,OH5_Ko,OH5_Nao,
     '          OH5_FractICaL,OH5_FractINaCa,
     '          OH5_DIFFCa,OH5_dNaCa,OH5_gama,OH5_nNaCa,
     '          OH5_kcyCa,OH5_kxcs,OH5_ksrCa,OH5_Cm,
     '          OH5_ALPHA12,OH5_BETA12
      COMMON /OH5_PARAM3/
     '  OH5_Cab,OH5_Ko,OH5_Nao,
     '  OH5_FractICaL,OH5_FractINaCa,
     '  OH5_DIFFCa,OH5_dNaCa,OH5_gama,OH5_nNaCa,
     '  OH5_kcyCa,OH5_kxcs,OH5_ksrCa,OH5_Cm,
     '  OH5_ALPHA12,OH5_BETA12
      REAL*8    OH5_F,OH5_R,OH5_T,
     '          OH5_Vecs,OH5_radius,OH5_length,
     '          OH5_Vup,OH5_Vrel,OH5_kmCa2,
     '          OH5_Mtrop,OH5_Ctrop,OH5_alfatrop,
     '          OH5_betatrop,OH5_gamatropSL,
     '          OH5_gamaSRSL,OH5_sacSL
      COMMON /OH5_PARAM4/
     '  OH5_F,OH5_R,OH5_T,
     '  OH5_Vecs,OH5_radius,OH5_length,
     '  OH5_Vup,OH5_Vrel,OH5_kmCa2,
     '  OH5_Mtrop,OH5_Ctrop,OH5_alfatrop,
     '  OH5_betatrop,OH5_gamatropSL,
     '  OH5_gamaSRSL,OH5_sacSL
      REAL*8    OH5_G_Ca_stretch,OH5_G_Na_stretch,OH5_G_K_stretch,
     '          OH5_G_Ns_stretch,OH5_G_An_stretch,
     '          OH5_E_Ns_stretch,OH5_E_An_stretch
      COMMON /OH5_PARAM5/
     '  OH5_G_Ca_stretch,OH5_G_Na_stretch,OH5_G_K_stretch,
     '  OH5_G_Ns_stretch,OH5_G_An_stretch,
     '  OH5_E_Ns_stretch,OH5_E_An_stretch
      REAL*8    OH5_alfaS,OH5_alfaV,
     '          OH5_alfaACh,OH5_betaACh,
     '          OH5_SLhst,OH5_IThst
      COMMON /OH5_PARAM6/
     '  OH5_alfaS,OH5_alfaV,
     '  OH5_alfaACh,OH5_betaACh,
     '  OH5_SLhst,OH5_IThst
      REAL*8 OH5_kdsdecay,OH5_Jsrleak
      COMMON /OH5_PARAM7/
     '  OH5_kdsdecay,OH5_Jsrleak
      !local variables
      REAL*8 VRTF,c1,c2,z1,z2 !SMAR009 21/12/98 ,z3,a1,b1,a2,a3,b3,a4,b4,b5,b6,
      REAL*8 V,mNa,hNa,dCa,fCa,f2Ca,xr1,xr2,xs,xACH,Nai,Ki,Cao,Cai,Cads,
     '  Caup,Carel,factivator,rto,qto !SMAR009 21/12/98 ,CaCALMOD,CaTROP,fproduct
      REAL*8 ATP,OH5_GKNa,ACh
      REAL*8 ENa,EK,EKs,ECa,Emh
      REAL*8 INa,IpNa,IbNa,IK1,IKr,IKs,IbK,IKATP,
     '  IKNa,IK,ICaLCa,ICaLK,ICaLNa,ICaLCads,ICaLKds,
     '  ICaLNads,IbCa,IKACH,INaK,INaCa,INaCads,Iup,
     '  Itr,Irel,XSRrel,Ito

      ENa = POTS(1)
      EK = POTS(2)
      EKs = POTS(3)
      ECa = POTS(4)
      Emh = POTS(5)

      ATP = 5.0d0
      OH5_GKNa = 0.0d0
      ACh = 0.0d0

! State variables
C ** DPN 04/11/98 - add f2Ca, slow ICaL inactivation
C ** DPN 05/11/98 - add Ito - r & q variables
      V          = y( 1) !membrane potential
      mNa        = y( 7) !Na channel m gate
      hNa        = y( 6) !Na channel h gate
      dCa        = y( 4) !Ca channel d gate
      fCa        = y( 5) !Ca channel f gate
      f2Ca       = y(11) !Ca channel slow inactivation gate
      xr1        = y(20) !fast K channel, fast rate
      xr2        = y(21) !fast K channel, slow rate
      xs         = y(22) !slow K channel
      xACh       = y(23) !??
      rto        = y(12) !inactivation variable for ito
      qto        = y(13) !activation variable for ito
      Nai        = y( 2) !intracellular [Na] 
      Ki         = y( 8) !intracellular [K]
      Cao        = y(16) !extracellular [Ca]
      Cai        = y( 3) !intracellular [Ca]
      Cads       = y(19) !sub-space [Ca]
      Caup       = y( 9) ![Ca]NSR
      Carel      = y(10) ![Ca]JSR
c      CaCALMOD   = y(14) ![Ca] bound to calmodulin
c      CaTROP     = y(15) ![Ca] bound to troponin
      factivator = y(17) !??? Ca SR release rate ???
c      fproduct   = y(18) !???

C      V          = y( 1) !membrane potential
C      mNa        = y( 7) !Na channel m gate
C      hNa        = y( 6) !Na channel h gate
C      dCa        = y( 4) !Ca channel d gate
C      fCa        = y( 5) !Ca channel f gate
C      xr1        = y(17) !fast K channel, fast rate
C      xr2        = y(18) !fast K channel, slow rate
C      xs         = y(19) !slow K channel
C      xACh       = y(20) !??
C      Nai        = y( 2) !intracellular [Na] 
C      Ki         = y( 8) !intracellular [K]
C      Cao        = y(13) !extracellular [Ca]
C      Cai        = y( 3) !intracellular [Ca]
C      Cads       = y(16) !sub-space [Ca]
C      Caup       = y( 9) ![Ca]NSR
C      Carel      = y(10) ![Ca]JSR
C      CaCALMOD   = y(11) ![Ca] bound to calmodulin
C      CaTROP     = y(12) ![Ca] bound to troponin
C      factivator = y(14) !??? Ca SR release rate ???
C      fproduct   = y(15) !???

C      V          = y( 1) !membrane potential
C      mNa        = y( 2) !Na channel m gate
C      hNa        = y( 3) !Na channel h gate
C      dCa        = y( 4) !Ca channel d gate
C      fCa        = y( 5) !Ca channel f gate
C      xr1        = y( 6) !fast K channel, fast rate
C      xr2        = y( 7) !fast K channel, slow rate
C      xs         = y( 8) !slow K channel
C      xACh       = y( 9) !??
C      Nai        = y(10) !intracellular [Na] 
C      Ki         = y(11) !intracellular [K]
C      Cao        = y(12) !intracellular [Ca]
C      Cai        = y(13) !extracellular [Ca]
C      Cads       = y(14) !sub-space [Ca]
C      Caup       = y(15) ![Ca]NSR
C      Carel      = y(16) ![Ca]JSR
C      CaCALMOD   = y(17) ![Ca] bound to calmodulin
C      CaTROP     = y(18) ![Ca] bound to troponin
C      factivator = y(19) !??? Ca SR release rate ???
C      fproduct   = y(20) !???

! Na currents
      INa   = OH5_GNa*mNa*mNa*mNa*hNa*(V-Emh)                  !eqn 36
C ** DPN 04/11/98 - new from HEART code
C      IpNa98  = OH5_GpNa/(1.d0+DEXP(0.125d0*(V+52.d0)))*(V-ENa)  !eqn 37
      IpNa  = OH5_GpNa/(1.d0+DEXP(-0.125d0*(V+52.d0)))*(V-ENa)  !eqn 37
C ** DPN 04/11/98 - new from HEART code
C      IbNa  = OH5_GbNa*Nai/140.d0*(V-ENa)                      !eqn 38
      IbNa  = OH5_GbNa*(V-ENa)

! K currents
c      IK1   = OH5_GK1*OH5_Ko/(OH5_Ko+OH5_kmK1)*(V-EK)          !eqn 29
c     '        /(1.d0+DEXP((V-EK-10.d0)/(RTF/2.0d0)))
c      IK1   = OH5_GK1*OH5_Ko/(OH5_Ko+OH5_kmK1)*(V-EK)          !eqn 29
c     '        /(1.d0+DEXP((V-EK+10.d0)/(RTF/2.0d0)))
      IK1   = OH5_GK1*OH5_Ko/(OH5_Ko+OH5_kmK1)*(V-EK)          !eqn 29
     '        /(1.d0+DEXP(1.25d0*(V-EK-10.d0)/RTF))
      IKr   = (OH5_GKr1*xr1+OH5_GKr2*xr2)*(V-EK)               !eqn 31
     '        /(1.d0+DEXP((V+9.d0)/22.4d0))         
      IKs   = OH5_GKs*xs*xs*(V-EKs)                            !eqn 32
      IbK   = OH5_GbK*(V-EK)                                   !eqn 33
      IKATP = OH5_GKATP*(V+80.d0)/(1.d0+(ATP/OH5_kATP)**2)     !eqn 34
      IKNa  = OH5_GKNa*(V-EK)*Nai/(Nai+OH5_kkNa)               !eqn 35
c      IK    = IKr+IKs+IKNa                               !eqn 30 
      IK    = IKr+IKs

! Transient outward current
      Ito = 0.005d0*rto*qto*Cai/(Cai+0.0005d0)*(V-EK)
c      Ito = 0.0d0
      
! Ca currents
C ** DPN 04/11/98 - new from HEART code
C      a1 = OH5_PCa*dCa*(1.d0-fCa)
C     '    *OH5_kcachoff/(OH5_kcachoff+Cai)
C      b1 = OH5_PCa*dCa*(1.d0-fCa)
C     '    *OH5_kdsoff/(OH5_kdsoff+Cads)
C      a2 = (V-50.d0)/RTF
C      a3 = DEXP(-a2)
C      b3 = DEXP(-a2/2.d0)
C      a4 = a2/(1.d0-a3)
C      b4 = a2/(1.d0-b3)
C      b5 = DEXP(1.d2/RTF)
C      b6 = DEXP(50.0d0/RTF)
C
C      ICaLCa   = 4.d0     *a1*b4*(Cai *b5-Cao*b3)             !eqn 39
C      ICaLK    = OH5_PCaK *a1*a4*(Ki  *b6-OH5_Ko *a3)         !eqn 40
Cc      ICaLK    = 1.0d0/oh5_pca *a1*b4*(Ki  *b6-OH5_Ko *a3)         !eqn 40
C      ICaLNa   = OH5_PCaNa*a1*a4*(Nai *b6-OH5_Nao*a3)         !eqn 41
Ccc      ICaLCads = 4.d0     *b1*b4*(Cads*b5-Cao*b3)             !eqn 42
Cc      ICaLKds  = OH5_PCaK *b1*a4*(Ki  *b6-OH5_Ko *a3)         !eqn 43
Ccc      ICaLKds  = OH5_PCaK *b1*b4*(Ki  *b6-OH5_Ko *a3)         !eqn 43
Ccc      ICaLNads = OH5_PCaNa*b1*a4*(Nai *b6-OH5_Nao*a3)         !eqn 44
C      
C      z3 = OH5_kcachoff/(OH5_kcachoff+Cai)
C
C      z1 = b4
C      ICaLK = dCa*(1.0d0-fCa)*z1*(Ki*b6-OH5_Ko*a3)*z3
C
C      z1 = b4
C      z2 = z1*(Cai*b5-Cao*b3)
C      ICaLCa = 4.0d0*OH5_PCa*dCa*(1.0d0-fCa)*z2*z3
C
C      z1 = OH5_PCaNa*OH5_PCa*a4
C      ICaLNa = dCa*(1.0d0-fCa)*z1*(Nai*b6-OH5_Nao*a3)*z3
      
      z1 = OH5_PCaK*OH5_PCa*(V-50.0d0)
     '  /(RTF*(1.0d0-DEXP(-(V-50.0d0)/RTF)))
      ICaLK = dCa*fCa*z1*(Ki*DEXP(50.0d0/RTF)-
     '  OH5_Ko*DEXP(-(V-50.0d0)/RTF))
      
      z1 = OH5_PCaNa*OH5_PCa*(V-50.0d0)
     '  /(RTF*(1.0d0-DEXP(-(V-50.0d0)/RTF)))
      ICaLNa = dCa*fCa*z1*(Nai*DEXP(50.0d0/RTF)-
     '  OH5_Nao*DEXP(-(V-50.0d0)/RTF))

      z1 = (V-50.0d0)/(RTF*(1.0d0-DEXP(-(V-50.0d0)/(0.5d0*RTF))))
      z2 = z1*(Cai*DEXP(50.0d0/(0.5d0*RTF))-
     '  Cao*DEXP(-(V-50.0d0)/(0.5d0*RTF)))
c      ICaLCa = 4.0d0*OH5_PCa*dCa*fCa*z2
c      z2 = z1*(Cads*DEXP(50.0d0/(0.5d0*RTF))-
c     '  Cao*DEXP(-(V-50.0d0)/(0.5d0*RTF)))
      ICaLCa = 4.0d0*OH5_PCa*dCa*fCa*z2

      ICaLCa = ICaLCa*f2Ca
      ICaLK  = ICaLK *f2Ca
      ICaLNa = ICaLNa*f2Ca
c      z1 = OH5_kcachoff/(OH5_kcachoff+Cai)
c      ICaLCa = ICaLCa*z1
c      ICaLK  = ICaLK *z1
c      ICaLNa = ICaLNa*z1

      IF (OH5_FractICaL.EQ.1.0d0) THEN
        ICaLCads = ICaLCa
        ICaLNads = ICaLNa
        ICaLKds  = ICaLK
        ICaLCa = 0.0d0
        ICaLNa = 0.0d0
        ICaLK  = 0.0d0
      ELSE
        write(*,*) 'BOB BOB'
c        ICaLCads = OH5_FractICaL * ICaLCa
c        ICaLNads = OH5_FractICaL * ICaLNa
c        ICaLKds  = OH5_FractICaL * ICaLK
c        ICaLCa = 0.0d0
c        ICaLNa = 0.0d0
c        ICaLK  = 0.0d0
      ENDIF

      IbCa     = OH5_GbCa*(V-ECa)                             !eqn 45
C MLB divide by zero
C      IKACh    = OH5_GKACh*OH5_Ko/(OH5_Ko+OH5_kmk1)*xACh      !eqn 46
C     '  /(1.d0+(OH5_kD/ACh)**2)
C     '  *(V-EK)/(1.d0+DEXP((V-EK-1.d1)*2.d0/RTF))
      IKAch = 0.0d0

! Pumps and exchanger currents
      INaK     = OH5_INaKmax*OH5_Ko /(OH5_Ko +OH5_kmK)
     '                      *Nai/(Nai+OH5_kmNa)

      VRTF = V/RTF 
      c1   = DEXP(OH5_gama*VRTF)
      c2   = DEXP((OH5_gama-1.d0)*VRTF)
C ** DPN 04/11/98 - new from HEART code
C      INaCa =   OH5_kNaCa*(c1*Nai**3*Cao - c2*OH5_Nao**3*Cai )  !eqn 48
C     '  /(1.d0+OH5_dNaCa*(   OH5_Nao**3*Cai +    Nai**3*Cao ))
C      INaCads = OH5_kNaCa*(c1*Nai**3*Cao - c2*OH5_Nao**3*Cads)  !eqn 49
C     '  / (1.d0+OH5_dNaCa*(   OH5_Nao**3*Cads+    Nai**3*Cao ))

      z1 = 1.0d0+OH5_dNaCa*(Cai*OH5_Nao**3+Cao*Nai**3)
      z2 = Nai**3*Cao*c1-OH5_Nao**3*Cai*c2
      INaCa = (1.0d0-OH5_FractINaCa)*OH5_kNaCa*z2/z1
      INaCa = INaCa/(1.0d0+Cai/0.0069d0)

      z1 = 1.0d0+OH5_dNaCa*(Cads*OH5_Nao**3+Cao*Nai**3)
      z2 = Nai**3*Cao*c1-OH5_Nao**3*Cads*c2
      INaCads = OH5_FractINaCa*OH5_kNaCa*z2/z1
      INaCads = INaCads/(1.0d0+Cads/0.0069d0)

! Ca sequestration
C ** DPN 04/11/98 - new from HEART code
C      Iup    = 3.d0*Cai-0.23d0*Caup*OH5_kcyCa*OH5_kxcs/OH5_ksrCa !eqn 50
C     '            /(Cai+       Caup*OH5_kcyCa*OH5_kxcs/OH5_ksrCa
C     '                             +OH5_kcyCa*OH5_kxcs+OH5_kcyCa)
c      Iup    = (0.4d0*Cai-0.03d0*Caup*OH5_kcyCa*OH5_kxcs/OH5_ksrCa) !eqn 50
c     '            /(Cai+       Caup*OH5_kcyCa*OH5_kxcs/OH5_ksrCa
c     '                             +OH5_kcyCa*OH5_kxcs+OH5_kcyCa)
      Iup    = (OH5_ALPHA12*Cai-
     '  OH5_BETA12*Caup*OH5_kcyCa*OH5_kxcs/OH5_ksrCa)
     '  /(Cai+       Caup*OH5_kcyCa*OH5_kxcs/OH5_ksrCa
     '  +OH5_kcyCa*OH5_kxcs+OH5_kcyCa)

      Itr     = 5.d1*(Caup-Carel)                                !eqn 51
c      Irel    = (factivator/(factivator+0.25d0))**2              !eqn 52
c     '  *OH5_kmCa2*Carel+OH5_Jsrleak*DEXP(OH5_gamaSRSL)
      Irel    = (factivator/(factivator+0.25d0))**2              !eqn 52
     '  *OH5_kmCa2*Carel+OH5_Jsrleak*Carel
      XSRrel  = Cai/(Cai+OH5_kmCa)                               !eqn 53
     '  +(1.d0-Cai/(Cai+OH5_kmCa))*Cads/(Cads+OH5_kmCads)

      I( 1) = INa
      I( 2) = IpNa
      I( 3) = IbNa
      I( 4) = IK1
      I( 5) = IKr
      I( 6) = IKs
      I( 7) = IbK
      I( 8) = IKATP
      I( 9) = IKNa
      I(10) = IK
      I(11) = ICaLCa
      I(12) = ICaLK
      I(13) = ICaLNa
      I(14) = ICaLCads
      I(15) = ICaLKds
      I(16) = ICaLNads
      I(17) = IbCa
      I(18) = IKACH
      I(19) = INaK
      I(20) = INaCa
      I(21) = INaCads
      I(22) = Iup
      I(23) = Itr
      I(24) = Irel
      I(25) = XSRrel
      I(26) = Ito

      RETURN
      END


      SUBROUTINE NOBLE98_RATES(V,Cai,ALPHA,BETA)

C#### Subroutine: NOBLE98_RATES
C###  Description:
C###    Calculates the gating rate constants for the Noble '98
C###    rat ventricular myocyte model.

      IMPLICIT NONE
      !passed variables
      REAL*8 V,Cai,ALPHA(9),BETA(9)
      !common blocks
      REAL*8    OH5_kcachoff,OH5_kmk1,OH5_kmK,
     '          OH5_kmNa,OH5_kNaCa,OH5_kmCa,
     '          OH5_k1,OH5_k2,OH5_k3,OH5_k4,
     '          OH5_kATP,OH5_KD,OH5_kkNa,
     '          OH5_kdsoff,OH5_kmCads,OH5_kdecay,
     '          OH5_Rdecay,OH5_INaKmax
      COMMON /OH5_PARAM1/
     '  OH5_kcachoff,OH5_kmk1,OH5_kmK,
     '  OH5_kmNa,OH5_kNaCa,OH5_kmCa,
     '  OH5_k1,OH5_k2,OH5_k3,OH5_k4,
     '  OH5_kATP,OH5_KD,OH5_kkNa,
     '  OH5_kdsoff,OH5_kmCads,OH5_kdecay,
     '  OH5_Rdecay,OH5_INaKmax
      REAL*8    OH5_GNa,OH5_GbK,OH5_GK1,OH5_GbNa,
     '          OH5_GpNa,OH5_GbCa,
     '          OH5_GKr1,OH5_GKr2,OH5_GKs,OH5_GKATP,OH5_GKACh,
     '          OH5_PCa,OH5_PCaK,OH5_PCaNa,OH5_PKNa
      COMMON /OH5_PARAM2/
     '  OH5_GNa,OH5_GbK,OH5_GK1,OH5_GbNa,
     '  OH5_GpNa,OH5_GbCa,
     '  OH5_GKr1,OH5_GKr2,OH5_GKs,OH5_GKATP,OH5_GKACh,
     '  OH5_PCa,OH5_PCaK,OH5_PCaNa,OH5_PKNa
      REAL*8    OH5_Cab,OH5_Ko,OH5_Nao,
     '          OH5_FractICaL,OH5_FractINaCa,
     '          OH5_DIFFCa,OH5_dNaCa,OH5_gama,OH5_nNaCa,
     '          OH5_kcyCa,OH5_kxcs,OH5_ksrCa,OH5_Cm,
     '          OH5_ALPHA12,OH5_BETA12
      COMMON /OH5_PARAM3/
     '  OH5_Cab,OH5_Ko,OH5_Nao,
     '  OH5_FractICaL,OH5_FractINaCa,
     '  OH5_DIFFCa,OH5_dNaCa,OH5_gama,OH5_nNaCa,
     '  OH5_kcyCa,OH5_kxcs,OH5_ksrCa,OH5_Cm,
     '  OH5_ALPHA12,OH5_BETA12
      REAL*8    OH5_F,OH5_R,OH5_T,
     '          OH5_Vecs,OH5_radius,OH5_length,
     '          OH5_Vup,OH5_Vrel,OH5_kmCa2,
     '          OH5_Mtrop,OH5_Ctrop,OH5_alfatrop,
     '          OH5_betatrop,OH5_gamatropSL,
     '          OH5_gamaSRSL,OH5_sacSL
      COMMON /OH5_PARAM4/
     '  OH5_F,OH5_R,OH5_T,
     '  OH5_Vecs,OH5_radius,OH5_length,
     '  OH5_Vup,OH5_Vrel,OH5_kmCa2,
     '  OH5_Mtrop,OH5_Ctrop,OH5_alfatrop,
     '  OH5_betatrop,OH5_gamatropSL,
     '  OH5_gamaSRSL,OH5_sacSL
      REAL*8    OH5_G_Ca_stretch,OH5_G_Na_stretch,OH5_G_K_stretch,
     '          OH5_G_Ns_stretch,OH5_G_An_stretch,
     '          OH5_E_Ns_stretch,OH5_E_An_stretch
      COMMON /OH5_PARAM5/
     '  OH5_G_Ca_stretch,OH5_G_Na_stretch,OH5_G_K_stretch,
     '  OH5_G_Ns_stretch,OH5_G_An_stretch,
     '  OH5_E_Ns_stretch,OH5_E_An_stretch
      REAL*8    OH5_alfaS,OH5_alfaV,
     '          OH5_alfaACh,OH5_betaACh,
     '          OH5_SLhst,OH5_IThst
      COMMON /OH5_PARAM6/
     '  OH5_alfaS,OH5_alfaV,
     '  OH5_alfaACh,OH5_betaACh,
     '  OH5_SLhst,OH5_IThst
      !local variables
!     SMAR009 23/12/98      REAL*8 alfa_f
      REAL*8 alfa_mNa,beta_mNa,alfa_hNa,beta_hNa,alfa_dCa,beta_dCa,
     '  alfa_fCa,beta_fCa,alfa_xr1,beta_xr1,alfa_xr2,beta_xr2,
     '  alfa_xs,beta_xs,alfa_xACH,beta_xACH,alfa_rto,beta_rto

! Na channel gating rate constants
      alfa_mNa = 2.d2*(V+41.d0)/(1.d0-DEXP(-0.1d0*(V+41.d0)))
      beta_mNa = 8.d3*DEXP(-0.056d0*(V+66.d0))
      alfa_hNa = 2.d1*DEXP(-0.125d0*(V+75.d0))
      beta_hNa = 2.d3/(1.d0+3.2d2*DEXP(-0.1d0*(V+75.d0)))

! Ca channel gating rate constants
C ** DPN 04/11/98 - new from HEART code
C      alfa_dCa = 90.d0*(V+19.d0)/(1.d0-DEXP(-0.25d0*(V+19.d0))) !eqn 4
C      beta_dCa = 36.d0*(V+19.d0)/(DEXP(0.1d0*(V+19.d0))-1.d0)   !eqn 4
C      alfa_f   = 12.d0/(1.d0+DEXP(-0.25d0*(V+34.d0)))           !eqn 5
C      alfa_fCa = alfa_f*(1.19d2*Cai/(OH5_Kcachoff+Cai)+1.d0)    !eqn 5
C      beta_fCa = 6.25d0*(V+34.d0)/(DEXP(0.25d0*(V+34.d0))-1.d0) !eqn 5
      alfa_dCa = 30.d0*(V+19.d0)/(1.d0-DEXP(-0.25d0*(V+19.d0))) !eqn 4
c      alfa_dCa = 30.d0*(V+24.d0)/(1.d0-DEXP(-0.25d0*(V+24.d0))) !eqn 4
      beta_dCa = 12.d0*(V+19.d0)/(DEXP(0.1d0*(V+19.d0))-1.d0)   !eqn 4
c      beta_dCa = 12.d0*(V+24.d0)/(DEXP(0.1d0*(V+24.d0))-1.d0)   !eqn 4
      alfa_fCa = 6.25d0*(V+34.d0)/(DEXP(0.25d0*(V+34.d0))-1.d0) !eqn 5
      beta_fCa = 12.d0/(1.d0+DEXP(-0.25d0*(V+34.d0)))           !eqn 5

! K  channel gating rate constants
      alfa_xr1 = 50.d0/(1.d0 +DEXP(-(V-5.d0)/9.d0))              !eqn 7
      beta_xr1 = 0.05d0*DEXP(-(V-20.d0)/15.d0)                  !eqn 7
      alfa_xr2 = alfa_xr1                                       !eqn 8
      beta_xr2 = 0.4d0*DEXP(-((V+30.d0)/30.d0)**3)              !eqn 8
      alfa_xs  = 14.d0/(1.d0+DEXP(-(V-40.d0)/9.d0))             !eqn 9
      beta_xs  = DEXP(-V/45.d0)                                 !eqn 9
      alfa_xACh= OH5_alfaACh                                    !eqn 10
      beta_xACh= OH5_betaACh                                    !eqn 10

! transient outward channel
      alfa_rto = 0.033d0*DEXP(-V/17.0d0)
      beta_rto = 33.0d0/(1.0d0+DEXP(-(V+10)/8.0d0))

      ALPHA(1) = alfa_mNa
      BETA(1)  = beta_mNa
      ALPHA(2) = alfa_hNa
      BETA(2)  = beta_hNa
      ALPHA(3) = alfa_dCa
      BETA(3)  = beta_dCa
      ALPHA(4) = alfa_fCa
      BETA(4)  = beta_fCa
      ALPHA(5) = alfa_xr1
      BETA(5)  = beta_xr1
      ALPHA(6) = alfa_xr2
      BETA(6)  = beta_xr2
      ALPHA(7) = alfa_xs
      BETA(7)  = beta_xs
      ALPHA(8) = alfa_xACH
      BETA(8)  = beta_xACH
      ALPHA(9) = alfa_rto
      BETA(9)  = beta_rto

      RETURN
      END


      SUBROUTINE OXSINI(IPARAM_1,IPARAM_2,LPARAM_2,RPARAM,RPARAM_2,Y)

C#### Subroutine: OXSINI
C###  Description:
C###    Initializes Oxsoft Heart parameters and variables.
C###    Note: OXSPREP is called after this to initialize preparation
C###    dependent variables.

      IMPLICIT NONE
      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs004.cmn'
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      INTEGER IPARAM_1(9),IPARAM_2(13)
      LOGICAL LPARAM_2(5)
      REAL*8 RPARAM(21,9),RPARAM_2(9),Y(*)
!     Local Variables
C      INTEGER I,NOPARA
      INTEGER L !,NOVARI
      !REAL*8 RESTART,TEMP,T0,TPLOT1,TPLOT2,TPLOT3,TSWITCH

      IF(DEBUG) write(*,'('' >>>call oxsini'')')

C *** Initialise Y variables

      DO L=1,33
        Y(L)=0.0d0
        ALPHA(L)=0.0d0
        BETA(L)=0.0d0
      ENDDO

C *** Initialize parameters

      CALL DEOXS1(IPARAM_1)                       !set control modes 
      CALL DEOXS2(IPARAM_2,LPARAM_2,RPARAM_2)     !set model modes
      CALL DEOXS3(1,RPARAM)                       !set Ca buffering
      CALL DEOXS3(2,RPARAM)                       !set Ca sequestration
      CALL DEOXS3(3,RPARAM)                       !set concentrations
      CALL DEOXS3(4,RPARAM)                       !set diffusion spaces
      CALL DEOXS3(5,RPARAM)                       !set ion channels
      CALL DEOXS3(6,RPARAM)                       !set other parameters
      CALL DEOXS3(7,RPARAM)                       !set pump exchange
      CALL DEOXS3(8,RPARAM)                       !set single channel
      CALL DEOXS3(9,RPARAM)                       !set time steps

C *** Set modes to default values

      CABUFF= .FALSE.
      BG= .FALSE.
      COUNTER=0

      BUFVOL=0.5d0
      CAVOL=1.0d0
      NCALMOD=3
      NCTROP=1
      NMTROP=2

C *** Initialize logical variables

      FAST =.FALSE.
      PLOTT=.FALSE.
      IF(MODE.EQ.1) THEN
        CLAMP=.FALSE.
      ELSE
        CLAMP=.TRUE.
      ENDIF
      IF(MODE.GT.2) THEN
        SLOW=.TRUE.
      ELSE
        SLOW=.FALSE.
      ENDIF

C *** Initialize timing and stimulus variables

      IF(MODE.NE.1) STIMSIZE=0.0d0
      DTS =0.5d0*DT
      DTSS=0.1d0*DTS
      DTL =DT
      IF(TPS.GT.TSTART) THEN
        STIM=0.0d0
      ELSE IF(TPS.LT.TSTART) THEN
        TPS=TEND+1.0d0
      ENDIF
      IF(STIMSIZE.EQ.0.0d0) TPS=TEND+1.0d0

C *** Initialise Y variables for version 2.0

      Y(20)=0.0005d0
      Y(21)=0.0015d0
      Y(22)=0.0d0
      ALPHA(26)=600
      BETA(26)=500
      ALPHA(27)=60
      BETA(27)=500
      Y(27)=0.0d0
      BETA(28)=1.0d0
      Y(28)=0.0d0
      ALPHA(12)=3.0d0
      BETA(12)=0.23d0
      Y(12)=0.3d0
      ALPHA(13)=50.0d0
      Y(13)=0.3d0
      ALPHA(29)=12000.0d0
      BETA(29)=100.0d0
      Y(29)=0.0d0
      ALPHA(30)=60.0d0
      BETA(30)=25.0d0
      Y(30)=0.0d0

      SHIFT(5)=0.0d0

C *** These variables are used when computing extracellular (Ca)

      DIFFCA=0.0005d0

C *** Data for calcium buffering from Robertson, Johnson & Potter,
C *** Biophysical Journal, (1981), 34, 559-569

      ALPHA(20)=100000.0d0
      BETA(20)=238.0d0
      ALPHA(21)=3900.0d0
      BETA(21)=19.6d0

C *** Data for mitochondrial calcium

      ALPHA(25)=1.25d0
      BETA(25)=0.15d0
      KCMREL=19.34d0
      KNMREL=11.36d0
      KCMUP=0.0877d0
      NNMREL=2.7d0
      NCMUP=1.4d0
      Y(25)=0.005d0

C *** Set default values for parameters common to all preps

      RS=0.0d0
      DO L=1,33
        SPEED(L)=1.0d0
        SHIFT(L)=0.0d0
      ENDDO
C dpn 17/02/98      TEMP=37.0d0
C dpn 17/02/98      T0=0.0d0
C dpn 17/02/98      TPLOT1 =10000.0d0
C dpn 17/02/98      TSWITCH=10000.0d0
C dpn 17/02/98      TPLOT2 =10000.0d0
C dpn 17/02/98      TPLOT3 =10000.0d0
C dpn 17/02/98      RESTART=10000.0d0
      KMINACT=0.001d0
      ALPHA(15)=10.0d0
      Y(16)=0.0d0
      Y(17)=1.0d0
      Y(18)=1.0d0
      Y(19)=0.0d0
      STIM =0.0d0

      Y(32)=0.0d0
      Y(33)=1.0d0

      RETURN
      END






      SUBROUTINE OXSPREP(Y)

C#### Subroutine: OXSPREP
C###  Description:
C###    Sets default parameters for various preparations.

      IMPLICIT NONE
c      INCLUDE 'cell/b12.cmn'
      INCLUDE 'cell/deoxs00.cmn'
      INCLUDE 'cell/oxs001.cmn'
      INCLUDE 'cell/oxs003.cmn'
      INCLUDE 'cell/oxs004.cmn'
      INCLUDE 'cell/oxs005.cmn'
!     Parameter List
      REAL*8 Y(*)
!     Local Variables
      INTEGER L
      REAL*8 TAU12,TAU13,TEMP,V!dpn - unused var's ,HM,S,PM

      IF(DEBUG) write(*,'('' >>>call oxsprep'')')

C *** Initialize parameters for particular preparation

      IF(PREP.EQ.1) THEN       !Purkinje fibre
        IF(DEBUG) write(*,'('' Prep is Purkinje fibre'')')
C Removed GBS 27-mar-96
c        C=0.075
c        GNA=750.0
c        PCA=15.0
c        IKM=180.0
c        GK1=920.0
c        KMK1=210.0
c        CAMODE=5
c        AMODE=4
c        CACT(4)=0.0005
c        GBNA=0.18
c        GBK=0.0
c        GFK=3.0
c        GFNA=3.0
c        GTO=1.0
c        GBCA=0.02
c        PUMP=125.0
c        KNACA=0.02
c        DNACA=0.001
c        KB=4.0
c        K=4.0
c        NAO=140.0
c        CAO=2.0
c        TOMODE=1
c        DX=5.0
c        PREPLENGTH=2.0
c        VOL=0.1
c        PF=0.7
        DVTEST=200.0d0
c        EC=-75.0
        TAU12=0.025d0  !Note: only used locally (also called TAUUP)
        TAU13=2.0d0    !Note: only used locally (also called TAUREPRIM)
c        TAUREL=0.05
c        KMCA=0.001
c        CANMODE=0
c        CA12=2.0       !Y(12)
c        CA13=1.0       !Y(13)

c        ALPHA(15)=5.0
c       BETA(15)=?      ****************************************

        SHIFT(5)=10.0d0
        SHIFT(7)=5.0d0

c        Y( 2)=8.0
c        Y( 4)=0.00005
c        Y( 5)=0.2
c        Y( 6)=0.01
c        Y( 7)=0.005
c        Y( 8)=1.0
c        Y( 9)=0.8
c        Y(10)=0.01
c        Y(11)=140.0
c        Y(14)=Y(8)
c        Y(15)=Y(8)

      ELSE IF (PREP.EQ.2) THEN !Ventricle
        IF(DEBUG) write(*,'('' Prep is Ventricle'')')
C       Not available yet
        DVTEST=6000.0d0
        TAU12=0.005d0  !Note: only used locally (also called TAUUP)
        TAU13=0.2d0    !Note: only used locally (also called TAUREPRIM)
        SHIFT( 5)=10.0d0
        SHIFT( 7)=5.0d0

      ELSE IF (PREP.EQ.3) THEN !Sinus node (rabbit)
        IF(DEBUG) write(*,'('' Prep is rabbit sinus node'')')
c        C=0.006
c        GNA=1.25
c        PCA=7.5
c        IKM=20.0
c        GK1=0.75
c        CAMODE=5
c        CACT(4)=0.0005
c        GBNA=0.07
c        GBK=0.0
c        GFK=6.0
c        GFNA=6.0
c        GTO=0.0
c        GBCA=0.01
c        PUMP=50.0
c        KNACA=0.002
c        DNACA=0.0001
c        KB=3.0
c        K=3.0
c        NAO=140.0
c        CAO=2.0
c        TOMODE=1
c        DX=8.0
c        PREPLENGTH=0.08
c        VOL=0.1
c        PF=1.0
        DVTEST=6000.0d0
c        EC=-60.0
        TAU12=0.005d0  !Note: only used locally (also called TAUUP)
        TAU13=0.2d0    !Note: only used locally (also called TAUREPRIM)
c        TAUREL=0.01
c        KMCA=0.002
c        INACAM=10000.0
c        KMODE=2
c        KMINACT=0.0005
c        CA12=1.98       !Y(12)
c        CA13=0.55       !Y(13)

c        ALPHA(15)=5.0
c       BETA(15)=?      ****************************************

        SPEED(5)=2.0d0

        SHIFT( 5)=10.0d0
        SHIFT( 7)=5.0d0
        SHIFT(14)=-30.0d0

c        Y( 2)=7.5
c        Y( 4)=0.000058
c        Y( 5)=0.007
c        Y( 6)=0.54
c        Y( 7)=0.0011
c        Y( 8)=0.785
c        Y( 9)=0.015
c        Y(10)=0.076
c        Y(11)=140.0
c        Y(14)=Y(8)
c        Y(15)=Y(8)

      ELSE IF (PREP.EQ.4) THEN !Single rat ventricular cell
        IF(DEBUG) write(*,'('' Prep is Single rat ventricular cell'')')
C       See Oxsoft Heart manual under preps.pas
        DVTEST=6000.0d0
        TAU12=0.005d0  !Note: only used locally (also called TAUUP)
        TAU13=0.2d0    !Note: only used locally (also called TAUREPRIM)
        SHIFT( 5)=10.0d0
        SHIFT( 7)=5.0d0

      ELSE IF (PREP.EQ.5) THEN !Single guinea-pig ventricular cell
        IF(DEBUG) 
     '    write(*,'('' Prep is Single guinea-pig ventricular cell'')')
C       See Oxsoft Heart manual under preps.pas
        DVTEST=6000.0d0
        TAU12=0.005d0  !Note: only used locally (also called TAUUP)
        TAU13=0.2d0    !Note: only used locally (also called TAUREPRIM)
        SHIFT( 5)=10.0d0
        SHIFT( 7)=5.0d0

      ELSE IF (PREP.EQ.6) THEN !Frog sinus venosus 
        IF(DEBUG) write(*,'('' Prep is Frog sinus venosus'')')
C       See Oxsoft Heart manual under preps.pas
        DVTEST=6000.0d0
        TAU12=0.005d0  !Note: only used locally (also called TAUUP)
        TAU13=0.2d0    !Note: only used locally (also called TAUREPRIM)
        SHIFT( 5)=10.0d0
        SHIFT( 7)=5.0d0

      ENDIF

C *** Initialize voltage and cleft

      Y(1)=EC
      ESAV=Y(1)+1.0d0
      K=4.0d0
      Y(3)=K !Need to define this somewhere (cleft K)
      DEPTH=10
      DO L=0,DEPTH-1
        KC(L)   =IDINT(K) !K
        KCE(L)  =IDINT(K) !K
        KCSAV(L)=IDINT(K) !K
      ENDDO
      KC(DEPTH)     =IDINT(KB) !KB
      KCE(DEPTH)    =IDINT(KB) !KB
      KCSAV(DEPTH)  =IDINT(KB) !KB
      KC(DEPTH+1)   =IDINT(KB) !KB
      KCE(DEPTH+1)  =IDINT(KB) !KB
      KCSAV(DEPTH+1)=IDINT(KB) !KB

C *** Initialize dependent variables defined by i/p parameters

      Y( 2)=NAI
      Y( 4)=CAI
      Y(11)=KI
      Y(12)=CA12
      Y(13)=CA13
      Y(14)=REPRIM
      Y(24)=CAO

C *** Initialize diffusion and space parameters

      TEMP=37.0d0
      IF(DABS(TORT).GT.1.E-6) THEN
        DIFF=1500.0d0/(TORT**2)
      ENDIF
      Z1=0.001d0*DEPTH*DX
      RTONF=(TEMP+273.0d0)*0.08554d0
      IF(SPACE.EQ.2) THEN
        V=3.14d0*1.33334d0*Z1**3
      ELSE
        V=3.14d0*PREPLENGTH*Z1**2
      ENDIF

C *** For Hilgemann allow for SR volume in calculating volumes

      IF(SRMODE.EQ.2) THEN
        VI=V*(1.0d0-VOL-V12-V13)
      ELSE
        VI=V*(1.0d0-VOL)
      ENDIF
      V  =V *VOL
      VF =V *965.d2
      VIF=VI*965.d2
      IF(DABS(C).GT.1.d-6) THEN
        CAP=1.0d0/C
      ENDIF
C dpn 07/05/98 ftnchk error
c      IF(DABS(DIFF).GT.1.d-6) THEN
c        S =DX*DX/DIFF
c      ENDIF
C dpn 06/04/98 ftnchek error        PM =S/DT
c      IF(DABS(DT).GT.1.d-6) THEN
c        PM =S/DT
c      ENDIF
C dpn 17/02/98      HM =2.0d0*(1.0d0+PM)

C *** Initialize mitochondrial system

      IF(MMODE.EQ.1) THEN
        VMIT=0.4d0*VI
        VI  =0.6d0*VI
        VIF =VI*965.d2
        ALPHA(25)=VI*ALPHA(25)
        BETA(25) =VI*BETA(25)
      ENDIF

C *** Initialize sarcoplasmic reticulum parameters

      IF(SRMODE.EQ.2) THEN
        SRVOL =V/VOL*V12
        RELVOL=V/VOL*V13
      ELSE
        SRVOL =VI*V12
        RELVOL=VI*V13
      ENDIF
      V12F=SRVOL *965.d2
      V13F=RELVOL*965.d2

      IF(SRMODE.EQ.1) THEN
        ALPHA(12)=2.0d0*VIF/(TAU12*CA12M)
        BETA(12) =1.d-6*ALPHA(12)*CA12M
        ALPHA(13)=2.0d0*V13F/TAU13
        BETA(13) =2.0d0*V13F/TAUREL
      ENDIF

C *** Initialize Ca buffers

      IF(BMODE.EQ.0) THEN
        DO L=20,23
          Y(L)=0.0d0
        ENDDO
      ELSE IF(BMODE.EQ.1) THEN
        Y(20)=(ALPHA(20)*Y(4))/(BETA(20)+ALPHA(20)*Y(4))
        Y(21)=(ALPHA(21)*Y(4))/(BETA(21)+ALPHA(21)*Y(4))
        Y(22)= ALPHA(22)*MG*(1-Y(4))/(ALPHA(22)*MG+BETA(22))
        Y(23)= ALPHA(23)*Y(4)+BETA(23)*ALPHA(22)*MG/BETA(22)+BETA(23)
        Y(23)= ALPHA(23)*Y(4)/Y(23)
      ELSE IF(BMODE.EQ.2) THEN
        Y(31)=Y(4)
        Y(22)=0.0d0
        Y(23)=0.0d0
      ENDIF

C *** Initialize external [Ca]

      IF(CAOMODE.EQ.1) THEN
        Y(24)=CAO
        CAB=CAO
      ELSE
        Y(24)=0.0d0
      ENDIF

      RETURN
      END


      REAL*8 FUNCTION PHI1(Lamda,Q0,Q1,Q2)

C#### Function: PHI1
C###  Type: REAL*8
C###  Description:
C###    Calculation of phi1 (eq 49).

      IMPLICIT NONE
!     Passed variables
      INTEGER Lamda
      REAL*8 Q0,Q1,Q2
!     Common blocks
      INTEGER    L
      COMMON /L/ L
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8      p,q,Q_0
      COMMON /PQ/ p,q,Q_0
      REAL*8 f1(9),g1(9),g2(9),f_dash0(9),f_dash1(9)
      COMMON /PARAMS1/ f1,g1,g2,f_dash0,f_dash1
!     Local variables
      INTEGER m !,IFAIL,IW(500)
      REAL*8  ARG !,ABSERR,W(2000)
      REAL*8   GG
c      EXTERNAL GG,Phi1_Integrand

      m=Muscle_type
      p=Q1/Q0
      ARG=Q2/Q0-(Q1/Q0)**2
      IF(ARG.GT.0.0d0) THEN
        q=DSQRT(ARG)
      ELSE
        write(*,'('' Error: -ve arg in Phi1 i.e. q is imaginary'')')
      ENDIF
      Q_0=Q0
      L=Lamda

      IF(Integration.EQ.1) THEN      !Analytic integration
        PHI1=Q0*f1(m)*(GG(L+1,(1.0d0-p)/q,p,q)-GG(L+1,-p/q,p,q))
C *** Remove numerical integration (temp???)
c      ELSE IF(Integration.EQ.2) THEN !Numerical integration
c        IFAIL=1
c        CALL D01AMF(Phi1_Integrand,0.D0,2,1.E-8,1.E-6,Phi1,ABSERR,
c     '    W,2000,IW,500,IFAIL)
c        IF(IFAIL.NE.0) THEN
c          WRITE(*,'('' IFAIL='',I1)') IFAIL
c        ENDIF
      ENDIF

      RETURN
      END


      REAL*8 FUNCTION PHI2(Lamda,Q0,Q1,Q2)

C#### Function: PHI2
C###  Type: REAL*8
C###  Description:
C###    Calculation of phi2 (eq 50).

      IMPLICIT NONE
!     Passed variables
      INTEGER Lamda
      REAL*8 Q0,Q1,Q2
!     Common blocks
      INTEGER    L
      COMMON /L/ L
      INTEGER    IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      COMMON /I/ IDOP,Model,Integration,Muscle_type,Test_type,Icouple
      REAL*8      p,q,Q_0 
      COMMON /PQ/ p,q,Q_0 
      REAL*8 f1(9),g1(9),g2(9),f_dash0(9),f_dash1(9)
      COMMON /PARAMS1/ f1,g1,g2,f_dash0,f_dash1
!     Local variables
      INTEGER m !,IFAIL,IW(500)
      REAL*8  ARG !,ABSERR,W(2000)
      REAL*8   GG
      !EXTERNAL GG,Phi2_Integrand

      m=Muscle_type
      p=Q1/Q0
      ARG=Q2/Q0-(Q1/Q0)**2
      IF(ARG.GT.0.0d0) THEN
        q=DSQRT(ARG)
      ELSE
        write(*,'('' Error: -ve arg in Phi2 i.e. q is imaginary'')')
      ENDIF
      Q_0=Q0
      L=Lamda

      IF(Integration.EQ.1) THEN      !Analytic integration
        IF(Lamda.EQ.0) THEN
          PHI2=Q0*( g2(m)* GG(0, -p/q   ,p,q) 
     '      + g1(m)*(GG(1,(1.0d0-p)/q,p,q)-GG(1,-p/q,p,q))
     '      + (f_dash0(m)-f_dash1(m))*(1.0d0-GG(0,(1.0d0-p)/q,p,q)) 
     '      + f_dash1(m)*(p-GG(1,(1.0d0-p)/q,p,q))) 
        ELSE IF(Lamda.EQ.1) THEN
          PHI2=Q0*( g2(m)* GG(1, -p/q   ,p,q) 
     '      + g1(m)*(GG(2,(1.0d0-p)/q,p,q)-GG(2,-p/q,p,q))
     '      +(f_dash0(m)-f_dash1(m))*(p-GG(1,(1.0d0-p)/q,p,q)) 
     '      + f_dash1(m)*(p*p+q*q-GG(2,(1.0d0-p)/q,p,q))) 
        ELSE IF(Lamda.EQ.2) THEN
          PHI2=Q0*( g2(m)* GG(2, -p/q   ,p,q) 
     '      + g1(m)*(GG(3,(1.0d0-p)/q,p,q)-GG(3,-p/q,p,q))
     '      +(f_dash0(m)-f_dash1(m))*(p*p+q*q-GG(2,(1.0d0-p)/q,p,q)) 
     '      + f_dash1(m)*(p*p*p+3.0d0*p*q*q-GG(3,(1.0d0-p)/q,p,q))) 
        ENDIF
C *** Remove numerical integration (temp??)
c      ELSE IF(Integration.EQ.2) THEN !Numerical integration
c        IFAIL=1
c        CALL D01AMF(Phi2_Integrand,0.D0,2,1.E-8,1.E-6,PHI2,ABSERR,
c     '    W,2000,IW,500,IFAIL)
c        IF(IFAIL.NE.0) THEN
c          WRITE(*,'('' IFAIL='',I1)') IFAIL
c        ENDIF
      ENDIF
      
      RETURN
      END


      REAL*8 FUNCTION TP1(L)

C#### Function: TP1
C###  Type: REAL*8
C###  Description:
C###    Passive tension-length relation for fibre axis

      IMPLICIT NONE
!     Parameter List
      REAL*8 L
!     Common Blocks
      REAL*8        a11,b11,k11
      COMMON /Tp_1/ a11,b11,k11 
!     Local Variables 
      REAL*8 e
         
      e=0.5d0*(L*L-1.d0)
      TP1= k11*e*(2.d0+b11*e/(a11-e))/(a11-e)**b11

      RETURN
      END


      REAL*8 FUNCTION TP2(L)

C#### Function: TP2
C###  Type: REAL*8
C###  Description:
C###    Passive tension-length relation for sheet axis

      IMPLICIT NONE
!     Parameter List
      REAL*8 L
!     Common Blocks
      REAL*8        a22,b22,k22
      COMMON /Tp_2/ a22,b22,k22 
!     Local Variables 
      REAL*8 e
         
      e=0.5d0*(L*L-1.d0)
      TP2= k22*e*(2.d0+b22*e/(a22-e))/(a22-e)**b22

      RETURN
      END


      REAL*8 FUNCTION TP3(L)

C#### Function: TP3
C###  Type: REAL*8
C###  Description:
C###    Passive tension-length relation for sheet normal

      IMPLICIT NONE
!     Parameter List
      REAL*8 L
!     Common Blocks
      REAL*8        a33,b33,k33
      COMMON /Tp_3/ a33,b33,k33 
!     Local Variables 
      REAL*8 e
         
      e=0.5d0*(L*L-1.d0)
      TP3= k33*e*(2.d0+b33*e/(a33-e))/(a33-e)**b33

      RETURN
      END


