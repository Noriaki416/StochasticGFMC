import sys
import os
import socket
import shutil
import subprocess

hostname = socket.gethostname()
print 'hostname: %s' % (hostname)
walltime = { 'i18cpu':'00:30:00', 'F4cpu':'24:00:00' }
if 'sekirei' in hostname:
    environment = 'sekirei'
#    queue = 'F4cpu'
    queue = 'i18cpu'
    print 'selected queue: %s' % queue
elif 'jupiter3' in hostname:
    environment = 'jupiter3'
elif 'jupiter4' in hostname:
    environment = 'jupiter4'
elif 'bowmore' in hostname:
    environment = 'bowmore'
else:
    print 'ERROR: unsupported environment.'
    sys.exit()

present_dir =os.getcwd()
print 'present dir: %s' % (present_dir)

temp_list = [ '0.05', '0.1', '0.15', '0.2', '0.25', '0.3', '0.5', '1.0', '1.5', '2.0' ]
#temp_list = [ '1.0', '1.5', '2.0' ]

p_list = []
for temp in temp_list:
    p = {'nsite': 32,
         't': '1.0',
         'v': '0.0',
         'mu': '0.0',
         'pot': '1.0',
         'potw': 1,
         'beta': 1.0/float(temp),
         'temp': temp,
         'nthermal': 2000,
         'nmeasure': 10,
         'nbin': 1000,
         'nseed': 1221,
         'nomega': 5 }
    p_list.append(p)

h = open('submit_all.sh','w')
h.write('#!/bin/sh\n')

count = 0
for p in p_list:
    header = 'conductance_nsite%i_temp%s_mu%s_pot%s' % ( p['nsite'], p['temp'], p['mu'], p['pot'] )
    flag1 = True
    flag2 = True
    while (flag1) or (flag2):
        count = count + 1
        parameter_filename = 'param.in.'+str(count)
        script_filename = 'job%i.sh' % (count)
        flag1 = os.path.exists(parameter_filename)
        flag2 = os.path.exists(script_filename)
        
    f = open(parameter_filename,'w')
    f.write('header %s\n' % header)
    f.write('nsite %i\n' % p['nsite'])
    f.write('t %s\n' % p['t'])
    f.write('v %s\n' % p['v'])
    f.write('mu %s\n' % p['mu'])
    f.write('pot %s\n' % p['pot'])
    f.write('potw %i\n' % p['potw'])
    f.write('beta %f\n' % p['beta'])
    f.write('nthermal %i\n' % p['nthermal'])
    f.write('nmeasure %i\n' % p['nmeasure'])
    f.write('nbin %i\n' % p['nbin'])
    f.write('nseed %i\n' % p['nseed'])
    f.write('nomega %i\n' % p['nomega'])
    f.close()
    print 'Wrote: %s' % (parameter_filename)

    g = open(script_filename,'w')
    g.write('#!/bin/sh\n')
    if environment == 'sekirei':
        g.write('#QSUB -queue %s\n' % queue)
        g.write('#QSUB -node 4\n')
        g.write('#QSUB -mpi 96\n')
        g.write('#QSUB -omp 1\n')
        g.write('#QSUB -place pack\n')
        g.write('#QSUB -over false\n')
        g.write('#PBS -l walltime=%s\n' % walltime[queue])
        g.write('cd %s\n' % (present_dir))
        g.write('mpijob ./run param.in.%i' % (count))
    elif environment == 'jupiter3':
        g.write('#PBS -l nodes=1:ppn=16\n')
#        g.write('NPROCS=`wc -l < $PBS_NODEFILE`\n')
        g.write('cd %s\n' % (present_dir))
        g.write('mpirun -np 16 ./run param.in.%i' % (count))
    elif environment == 'jupiter4':
        g.write('#PBS -l nodes=1:ppn=8\n')
        g.write('NPROCS=`wc -l < $PBS_NODEFILE`\n')
        g.write('cd %s\n' % (present_dir))
        g.write('mpirun -np 8 ./run param.in.%i' % (count))
    elif environment == 'bowmore':
        g.write('#$ -S /bin/sh\n')
        g.write('#$ -pe openmpi 24\n')
        g.write('#$ -binding pe linear:24\n')
        g.write('#$ -v OMP_NUM_THREADS=1\n')
        g.write('#$ -cwd -m n\n')
        g.write('. ~/.bashrc\n')
        g.write('cat $TMPDIR/machines | sort | uniq > $JOB_NAME.m$JOB_ID\n')
        g.write('mpirun -np 24 /home/k-yoshimi/program/chargeglass_box/chargeglass_kato/chargeglass/a.out param.in.%i;date\n' % (count))
    g.close()
    print 'Wrote: %s' % (script_filename)
    h.write('qsub job%i.sh\n' % (count))
print 'Wrote: submit_all.sh'
h.close()
subprocess.call(['chmod','770','submit_all.sh'])
