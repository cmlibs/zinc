      SUBROUTINE E04UEF_NOPRINT()
C
      IMPLICIT NONE
C
      CALL E04UEF('Print Level 0')
C
      RETURN
      END

      SUBROUTINE E04UEF_FUNC_PREC()
C
      IMPLICIT NONE
C
      CALL E04UEF('Function Precision 1.0E-6')
C
      RETURN
      END

      SUBROUTINE E04UEF_DEFAULTS()
C
      IMPLICIT NONE
C
      CALL E04UEF('Defaults')
C
      RETURN
      END

      SUBROUTINE E04UEF_VERIFY()
C
      IMPLICIT NONE
C
      CALL E04UEF('Verify Level 3')
C
      RETURN
      END

      SUBROUTINE E04UEF_MAX_ITERS_2000()
C
      IMPLICIT NONE
C
      CALL E04UEF('Major Iteration Limit 2000')
      CALL E04UEF('Minor Iteration Limit 2000')
C
      RETURN
      END

      SUBROUTINE E04UEF_MAX_ITERS_10()
C
      IMPLICIT NONE
C
      CALL E04UEF('Major Iteration Limit 10')
      CALL E04UEF('Minor Iteration Limit 10')
C
      RETURN
      END

