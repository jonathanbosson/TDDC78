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
  double precision,dimension(n)       :: tmp1,tmp2
  double precision                    :: error,x
  real                                :: t1,t0
  integer                             :: i,j,k, threadId, numThreads
  character(len=20)                   :: str
  
  ! Save the next iteration to process the row
  integer, dimensions(n+1)            :: rowNextIteration

  ! Set boundary conditions and initial values for the unknowns
  T=0.0D0
  T(0:n+1 , 0)     = 1.0D0
  T(0:n+1 , n+1)   = 1.0D0
  T(n+1   , 0:n+1) = 2.0D0
  

  ! Solve the linear system of equations using the Jacobi method
  t0 = omp_get_wtime()

  do i=1,n+1
    rowNextIteration(i)=1
  end do
  !$omp end do
  !$omp end parallel

  !iteration counter, shared
  iterations=1

  !$omp parallel default(private) shared(iterations, row_next_iteration, T)
  !$omp do schedule ( STATIC, 1 )
  do k=1, maxiter
     threadId = omp_get_thread_num()

     tmp1=T(1:n,0)
     error=0.0D0

     do j=1,n
        if(j /= n) then
           !wait for previous iteration to finish, works like a lock
           do while (row_next_iteration(j+1) < k)
              cycle
           end do
        end if

        tmp2=T(1:n,j)
        T(1:n,j)=(T(0:n-1,j)+T(2:n+1,j)+T(1:n,j+1)+tmp1)/4.0D0
        error=max(error,maxval(abs(tmp2-T(1:n,j))))
        tmp1=tmp2   

        !No critical section needed due to only one thread per row
        row_next_iteration(j) = k + 1
     end do

     !$omp critical
     iterations = iterations + 1
     !$omp end critical

  end do
  !$omp end do
  !$omp end parallel

  t1 = omp_get_wtime()


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
