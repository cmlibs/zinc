! cell_reserved.inc

C *** Y,DY array reserved positions
      INTEGER Vm
      
      PARAMETER(            Vm =  1 ) !

C *** PARAMETERS array reserved positions
      INTEGER Cm,AM
      
      PARAMETER(            Cm =  1 ) !membrane capacitance
      PARAMETER(            Am =  2 ) !membrane area

C *** PROTOCOL array reserved positions
      INTEGER Is1current,Is1start,Is1stop,Is2current,Is2start,
     '  Is2stop,IsFreqDuration,IsFreqMag,IsFreqPeriod,PseudoIs
      
      PARAMETER(      PseudoIs =  1 ) !Pseudo stimulus current (uA/mm^3)
      PARAMETER(      Is1start =  2 ) !Stimulus 1 time on (ms)
      PARAMETER(       Is1stop =  3 ) !Stimulus 1 time off (ms)  
      PARAMETER(    Is1current =  4 ) !Stimulus 1 magnitude (uA/mm^3)
      PARAMETER(      Is2start =  5 ) !Stimulus 2 time on (ms) 
      PARAMETER(       Is2stop =  6 ) !Stimulus 2 time off (ms)
      PARAMETER(    Is2current =  7 ) !Stimulus 2 magnitude (uA/mm^3)
      PARAMETER(  IsFreqPeriod =  8 ) !Stimulus frequency period (ms) 
      PARAMETER(IsFreqDuration =  9 ) !Stimulus frequency duration (ms) 
      PARAMETER(     IsFreqMag = 10 ) !Stimulus frequency stimulus magnitude (uA/mm^3)

C *** SIZES array positions - ?? Unused ???
      INTEGER NUM_EQN,NUM_CONTROL,NUM_MODEL,NUM_DERIVED,NUM_PARAM,
     '  NUM_PROTOCOL,NUM_AII,NUM_AIO,NUM_ARI,NUM_ARO,NUM_TIME
      
      PARAMETER(       NUM_EQN =  1 ) !
      PARAMETER(   NUM_CONTROL =  2 ) !
      PARAMETER(     NUM_MODEL =  3 ) !
      PARAMETER(   NUM_DERIVED =  4 ) !
      PARAMETER(     NUM_PARAM =  5 ) !
      PARAMETER(  NUM_PROTOCOL =  6 ) !
      PARAMETER(       NUM_AII =  7 ) !
      PARAMETER(       NUM_AIO =  8 ) !
      PARAMETER(       NUM_ARI =  9 ) !
      PARAMETER(       NUM_ARO = 10 ) !
      PARAMETER(      NUM_TIME = 11 ) !

C *** CONTROL array reserved positions
      INTEGER ODE

      PARAMETER(ODE=1) !==1 if evaluating ODEs in RHS Routine

C *** The TIME array, only ever has reserved parameters!!
      INTEGER TCell,DTCell
      PARAMETER(TCell=1) !The current time for the cell model
      PARAMETER(DTCell=2) !The current time step size for the cell model
