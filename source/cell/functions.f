      REAL*8 FUNCTION ERRORC(X)

C#### Function: ERRORC
C###  Type: REAL*8
C###  Description:
C###    Returns the complementary error function of X (erfc(x))
C###

      IMPLICIT NONE
!     Passed parameters
      REAL*8 X
!     Local variables
      REAL*8 T,Z

      Z = DABS(X)
      T = 1.0D0/(1.0D0+0.5D0*Z)
      ERRORC = T*DEXP(-Z*Z-1.26551223D0+T*(1.00002368D0+T*0.37409196D0+
     '  T*(0.09678418D0+T*(-0.18628806D0+T*(0.27886807D0+
     '  T*(-1.13520398D0+T*(1.48851587D0+T*(-0.82215223D0+
     '  T*0.17087277D0))))))))
      IF(X.LT.0.0D0) ERRORC = 2.0D0 - ERRORC

      RETURN
      END


