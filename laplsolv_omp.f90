program laplsolv
!-----------------------------------------------------------------------
! Serial program for solving the heat conduction problem
! on a square using the Jacobi method.
! Written by Fredrik Berntsson (frber@math.liu.se) March 2003
! Modified by Berkant Savas (besav@math.liu.se) April 2006
!-----------------------------------------------------------------------
use omp_lib
  
  integer, parameter                   :: n = 1000, maxiter = 1000
  character(len=20), parameter         :: filename = "measures.csv"
  double precision, parameter          :: tol = 1.0E-3
  double precision, dimension(0:n+1, 0:n+1) :: T
  double precision, dimension(n)       :: tmp1, tmp2, tmp3
  double precision                     :: error,x,t0,t1
  integer                              :: j, k, numK, rowLast, rowFirst
  character(len = 20)                  :: str
  logical                              :: exist

  ! Set boundary conditions and initial values for the unknowns
  T = 0.0D0
  T(0:n+1 , 0)     = 1.0D0
  T(0:n+1 , n+1)   = 1.0D0
  T(n+1   , 0:n+1) = 2.0D0


  ! Solve the linear system of equations using the Jacobi method
  t0 = omp_get_wtime()

  !$omp parallel private(j, k, tmp1, tmp2, tmp3, rowFirst, rowLast) shared(T, error)

  rowLast = (omp_get_thread_num() + 1) * (n / omp_get_num_threads())
  rowFirst = omp_get_thread_num() * (n / omp_get_num_threads())

  do k = 1, maxiter
    error = 0.0D0

    tmp1 = T(1:n, rowFirst)
    tmp3 = T(1:n, rowLast)

    !$omp barrier
    !$omp do reduction(max : error)
    do j = 1, n
      tmp2 = T(1:n, j)

      if (j == rowLast) then
        T(1:n, j) = ( T(0:n-1, j) + T(2:n+1, j) + tmp3 + tmp1 )/4.0D0
      else
        T(1:n, j) = ( T(0:n-1, j) + T(2:n+1, j) + T(1:n, j+1) + tmp1 )/4.0D0
      end if

      error = max( error, maxval(abs(tmp2 - T(1:n, j))) )
      tmp1 = tmp2
    end do

    if (error < tol) then
      numK = k
      exit
    end if
    !$omp barrier

  end do
  !$omp end parallel
  
  t1 = omp_get_wtime()

  write(unit=*, fmt=*) 'Time:', t1 - t0, 'Number of Iterations:', numK
  write(unit=*, fmt=*) 'Temperature of element T(1,1)  = ', T(1, 1)

  inquire(file=filename, exist=exist)
  if (exist) then
    open(12, file=filename, status="old", position="append", action="write")
  else
    open(12, file=filename, status="new", action="write")
  end if

  write(12, *) omp_get_max_threads(),t1-t0
  close(12)

end program laplsolv