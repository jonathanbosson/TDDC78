program laplsolv
!-----------------------------------------------------------------------
! Serial program for solving the heat conduction problem 
! on a square using the Jacobi method. 
! Written by Fredrik Berntsson (frber@math.liu.se) March 2003
! Modified by Berkant Savas (besav@math.liu.se) April 2006
!-----------------------------------------------------------------------

use omp_lib

  integer, parameter                  :: n=1000, maxiter=1000
  double precision,parameter          :: tol=1.0E-3
  double precision,dimension(0:n+1,0:n+1) :: T
  double precision,dimension(n)       :: tmpLeft, tmpMid, tmpRight,tmpLast
  double precision                    :: error,x
  real                                :: t1,t0
  integer                             :: j,k, numThreads
  integer                             :: myid, teamSize, evCols, myCols
  character(len=20)                   :: str, arg


  ! Set boundary conditions and initial values for the unknowns
  T=0.0D0
  T(0:n+1 , 0)     = 1.0D0
  T(0:n+1 , n+1)   = 1.0D0
  T(n+1   , 0:n+1) = 2.0D0
  

  !call getarg(1, arg)
  !read(arg, '(i10)' ) numThreads
  !call omp_set_num_threads(numThreads)

  numThreads = omp_get_max_threads()

  call cpu_time(t0)
  !t0 = omp_get_wtime()

  ! Solve the linear system of equations using the Jacobi method
  do k=1, maxiter
    error = 0.0D0

    !$OMP parallel default(private) shared(T,k) reduction(max:error)
    myid = omp_get_thread_num()
    teamSize = omp_get_num_threads()
    evCols = n/teamSize

    tmpLeft = T(1:n, myid*evCols)
    if(myid == teamSize-1) then
      tmpLast = T(1:n, n+1)
      myCols = evCols + mod(n, teamSize)
    else
      myCols = evCols
      tmpLast = T(1:n,(myid+1)*evCols+1)
    end if

    !$OMP BARRIER
    do j=myid*evCols+1, myid*evCols+myCols
      tmpMid = T(1:n, j)
      if (j == (myid+1)*myCols) then
        tmpRight = tmpLast
      else
        tmpRight = T(1:n, j+1)
      end if
      T(1:n, j) = ( T(0:n-1, j) + T(2:n+1, j) + tmpRight + tmpLeft )/4.0D0
      error = max( error, maxval( abs(tmpMid - T(1:n, j)) ) )
      tmpLeft = tmpMid
    end do
    !$OMP end parallel

    if (error < tol) then
      exit
    end if
  end do

  !t1 = omp_get_wtime()
  call cpu_time(t1)


  !write(unit=*, fmt=*) 'Time:',t1-t0
  write(unit=*,fmt=*) 'Time:',t1-t0,'Number of Iterations:',k
  write(unit=*,fmt=*) 'Temperature of element T(1,1)  =',T(1,1)

  ! Uncomment the next part if you want to write the whole solution
  ! to a file. Useful for plotting. 
  
  !open(unit=7,action='write',file='result.dat',status='unknown')
  !write(unit=str,fmt='(a,i6,a)') '(',N,'F10.6)'
  !do i=0,n+1
  !   write (unit=7,fmt=str) T(i,0:n+1)  
  !end do
  !close(unit=7)
  
end program laplsolv
