import sys
import subprocess
import time

t0 = time.time()

t = '1.0'
mu = '-1.0'
pot = '1.0'
nthermal = 2000
nmeasure = 10
nbin = 1000
seed = 1121
nsite = 32
nomega = 5

temps = 0.05
tempe = 1.0
tempn = 20
temph = (tempe-temps)/(tempn-1)
sys.stdout.flush()

for i in xrange(tempn):    
    temp = temps + temph*i
    f = open('param.in','w')
    f.write('nsite %i\n' % (nsite))
    f.write('t %s\n' % (t))
    f.write('v 0.0\n')
    f.write('mu %s\n' % (mu))
    f.write('pot %s\n' % (pot))
    f.write('potw 1\n')
    f.write('beta %f\n' % (1./temp))
    f.write('nthermal %i\n' % (nthermal))
    f.write('nmeasure %i\n' % (nmeasure))
    f.write('nbin %i\n' % (nbin))
    f.write('nseed %i\n' % (seed))
    f.write('nomega %i\n' % (nomega))
    f.close()
    p = subprocess.Popen(['mpirun -np 16 ./qmc param.in'], shell=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
    stdout_data, stderr_data = p.communicate()
    print('%f %s' % (temp, stdout_data))

t1 = time.time()
print("# running time ="+str(t1-t0)+"[s]")
