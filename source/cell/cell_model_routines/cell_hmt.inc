! cell_hmt.inc
!  Array elements for the HMT cardiac mechanics model.

C *** State (Y and DY) arrays
C ***   ODE's must come first !!!!
      INTEGER Cab,z,ExtensionRatio,Tension,IsometricTension,Cai,
     '  ExtensionRatio_prev,phi1,phi2,phi3
      !PARAMETER(Vm=1) [reserved]
      PARAMETER(Cab=2) !Conc. of Ca++ bound to troponin-C (nmol.mm^-3 = mM)
      PARAMETER(z=3) !Proportion of actin sites avilable for binding ()
      PARAMETER(ExtensionRatio=4) !Extension ratio ()
      PARAMETER(Tension=5) !Active fibre tension (mN.mm^-2 = kPa)
      PARAMETER(IsometricTension=6) !Isometric tension (kPa)
      PARAMETER(Cai=7) !Intracellular [Ca++] (mM)
      PARAMETER(ExtensionRatio_prev=8) !Extension ratio at previous time ()
      PARAMETER(phi1=9)  !Hereditary integral for 1st time constant ()
      PARAMETER(phi2=10) !Hereditary integral for 2nd time constant ()
      PARAMETER(phi3=11) !Hereditary integral for 3rd time constant ()

C *** CURRENTS and RATES array (DERIVED??)

C *** MODEL array
      INTEGER CALCULATE_Cai
      PARAMETER(CALCULATE_Cai=1) !Switch on/off calculation of Cai within HMT RHS routine.

C *** CONTROL array
      INTEGER STEADY_STATE
c      PARAMETER(ODE=1) !cell_reserved.inc
      PARAMETER(STEADY_STATE=2) !Switch between steady state and dynamic equations


C *** PARAMETERS array
      INTEGER Ca_max,Ca_min,Ca_tau,Rho0,Rho1,Tm_n_0,Tm_p50_0,beta1,
     '  beta2,beta0,Tref,gamma,Cab_max,alfa0,a,alpha1,alpha2,alpha3,A1,
     '  A2,A3,MaxExtRatio,MinExtRatio
      PARAMETER(Rho0=1) !Ca++ binding to Tn-C on rate constant (ms^-1.mM^-1)
      PARAMETER(Rho1=2) !Ca++ binding to Tn-C off rate constant (ms^-1)
      PARAMETER(Cab_max=3) !Max conc of Ca++ initially bound to Tn-C (mM)
      PARAMETER(alfa0=4) !Tropomyosin rate constant (ms^-1)

      PARAMETER(Tm_n_0=6) !Hill parameter (ref value for n) ()
      PARAMETER(Tm_p50_0=7) !Hill parameter (ref value for pC50) ()

      PARAMETER(a=17) !Fading memory curvature ()
      PARAMETER(alpha1=18) !Rate constant associated with the first order slow tension recovery (ms^-1)
      PARAMETER(alpha2=19) !Rate constant associated with the second order fast tension recovery (ms^-1)
      PARAMETER(alpha3=20) !Rate constant associated with the second order fast tension recovery (ms^-1)
      PARAMETER(A1=21) !Weighting coefficient associated with the first order slow tension recovery ()
      PARAMETER(A2=22) !Weighting coefficient associated with the second order fast tension recovery ()
      PARAMETER(A3=23) !Weighting coefficient associated with the second order fast tension recovery ()
      PARAMETER(Tref=24) !Reference tension (kPa)
      PARAMETER(beta0=25) !Isometic length dependence ()
      PARAMETER(beta1=26) !Hill n length dependence ()
      PARAMETER(beta2=27) !Hill pC50 length dependence ()
      PARAMETER(Ca_max=28) ![Ca++] twitch maximum (mM)
      PARAMETER(Ca_min=29) ![Ca++] twitch maximum (mM)
      PARAMETER(Ca_tau=30) ![Ca++] twitch time constant (ms)
      PARAMETER(gamma=31) !Scaling factor for To ?? ()
      PARAMETER(MaxExtRatio=32) !Maximum allowable extension ratio
      PARAMETER(MinExtRatio=33) !Minimum allowable extension ratio

C *** PROTOCOL array

C *** AII array

C *** AIO array

C *** ARI array

C *** ARO array
