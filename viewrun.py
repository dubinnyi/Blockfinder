#!/usr/bin/python3 -u

import argparse
import re
import math
import os, sys, stat

task_new_re= re.compile('Task(?P<task>\d+) +start= +(?P<start>(\d+ +)+) +end= +(?P<end>(\d+ +)+)')
task_old_re= re.compile(' *(?P<start>(\d+ +)+) +: +(?P<end>(\d+ +)+)')

def scan_tasks(taskfile):
   num_tasks = 0
   tasks = []
   with open(taskfile, "r") as task:
      for i, line in enumerate(task):
         tm_new = task_new_re.match(line)
         tm_old = task_old_re.match(line)
         if tm_new:
            tm = tm_new
            task_id = tm.group('task')
         elif tm_old:
            tm = tm_old
            task_id = num_tasks
         else:
            str= "Error in taskfile {}, line {}:\n{}".format(taskfile, i, line)
            print(str, file=sys.stderr)
            task = {}
            continue
         task= {'task' : task_id, 'start': tm.group('start'), 'end': tm.group('end'), 'line': line.strip() } 
         num_tasks+= 1
         tasks.append(task)
   return(tasks)

task_run_re1= re.compile('^\\[Task(?P<task>\d+)\\] +(?P<iter>\d+)M +(?P<time>\d+) sec +(?P<speed>\d+\.\d+) +Kiter\\/sec +max_P=(?P<maxP>\d+) +ELB_found= (?P<ELB>\d+)')
task_run_re2= re.compile('^\\[Task(?P<task>\d+)\\] +(?P<iter>\d+)M +(?P<time>\d+) sec +(?P<speed>\d+\.\d+) +Kiter\\/sec +((?P<codespeed>\d+\.\d+) +Mcodes\\/sec)? +max_P=(?P<maxP>\d+) +ELB_found= (?P<ELB>\d+)')
task_run_re = task_run_re2
#[Task00174]     9M        8 sec  973.97 Kiter/sec  100.34 Mcodes/sec max_P=10 ELB_found= 0
task_finished_re = re.compile('^\\[Task(?P<task>\d+)\\] finished task (?P<task2>\d+) after (?P<iter>\d+) iterations')

def scan_log(args, tasks):
   logfiles_list = args.log
   restart       = args.restart
   savelog       = args.save_log if 'save_log' in args else None
   n_tasks = len(tasks)
   log_lines_in_all_files = 0
   list_task_iter  = [None for t in range(n_tasks)]
   list_task_time  = [None for t in range(n_tasks)]
   list_task_speed = [None for t in range(n_tasks)]
   list_task_maxP  = [None for t in range(n_tasks)]
   list_task_ELB   = [None for t in range(n_tasks)]
   list_last_log   = [ ["", ""]  for t in range(n_tasks)]
   list_task_finished = [False for t in range(n_tasks)]

   wall_time = 0

   for logfile in logfiles_list:

      st = os.stat(logfile)
      wall_time += st.st_mtime-st.st_atime

      print("Start {} scan".format(logfile))
      with open(logfile, "r") as log:
        log_lines = 0
        for line in log:
           log_lines += 1
           log_lines_in_all_files += 1
           task_run_match = task_run_re1.match(line)
           if not task_run_match:
              task_run_match = task_run_re2.match(line)
           if task_run_match:
              task_num   =  int(task_run_match.group('task'))
              task_iter  =  int(task_run_match.group('iter'))*1000000
              task_sec   =  int(task_run_match.group('time'))
              task_speed = float(task_run_match.group('speed'))*1000
              task_maxP  =  int(task_run_match.group('maxP'))
              task_ELB   =  int(task_run_match.group('ELB'))
              list_task_iter[task_num]  = task_iter
              list_task_time[task_num]  = task_sec
              list_task_speed[task_num] = task_speed
              list_task_maxP[task_num]  = task_maxP
              list_task_ELB[task_num]   = task_ELB
              list_last_log[task_num][0] = list_last_log[task_num][1] 
              list_last_log[task_num][1] = line.strip() 
              continue
           task_finished_match = task_finished_re.match(line)
           if task_finished_match:
              task_num =  int(task_finished_match.group('task'))
              task_iter = int(task_finished_match.group('iter'))
              list_task_iter[task_num] = task_iter
              list_task_finished[task_num] = True
              list_last_log[task_num][0] = list_last_log[task_num][1] 
              list_last_log[task_num][1] = line.strip() 
              
        print("Scan of log file {} finished, {:,} lines scanned".format(logfile, log_lines))
        
   print("Scan of all {} log files finished, {:,} lines scanned".\
      format(len(logfiles_list), log_lines_in_all_files))

   total_iter = 0
   total_finished = 0
   total_none = 0
   total_running = 0
   total_time = 0
   total_ELB = 0
   global_maxP = 0

   out_line_len = 100
   out_n_lines = math.floor(n_tasks/out_line_len) + 1
   out_line = "{:>5} ".format(1)
   out_line_num = 0
   
   res= open(restart, 'w')
   if savelog:
      slog = open(savelog, 'w')
   else:
      slog = None

   for task_num in range(n_tasks):
      
      if not ( task_num % out_line_len ) and not task_num == 0:
         out_line += " {:>5}".format(task_num)
         print(out_line)
         out_line_num += 1
         out_line = ""
         out_line += "{:>5} ".format(task_num+1)

      if list_task_time[task_num]:
         total_time += list_task_time[task_num]

      if list_task_ELB[task_num]:
         total_ELB  += list_task_ELB[task_num]

      if list_task_maxP[task_num] and list_task_maxP[task_num] > global_maxP:
         global_maxP = list_task_maxP[task_num]

      if list_task_iter[task_num]:
         total_iter += list_task_iter[task_num]
         if not list_task_finished[task_num]:
            total_running += 1
            out_line += "*"
      else:
         total_none += 1
         out_line += "."

      if list_task_finished[task_num]:
         total_finished += 1
         out_line += "F"
      else:
         if res:
            print(tasks[task_num]['line'], file=res)
      if slog:
         for line in list_last_log[task_num]:
           if line:
             print(line, file=slog)
   
   rest_size = ( n_tasks ) % out_line_len
   out_line_fill= out_line + (" " * (out_line_len - rest_size ) )+ " {:>5}".format(n_tasks)
   print(out_line_fill)
      
   speed_list = []
   for task_num in range(n_tasks):
      if not list_task_finished[task_num] and list_task_speed[task_num]:
         speed_list.append(list_task_speed[task_num])

   average_speed = sum(speed_list)/len(speed_list) if len(speed_list) else 0
    
   print("Task status: {} finished, {} running and {} waiting ".format(total_finished, total_running, total_none))
   print("Total {:,} iterations in all tasks".format(total_iter))
#   print("Total {:,} wall clock time".format(wall_time))
   print("Total {:,} seconds in all tasks".format(total_time))
   print("Total {:,} ELB in all tasks".format(total_ELB))
   print("Average speed is {:.2f} Kiter/sec in running tasks".format(average_speed/1000))
   print("Global max_P = {} in all tasks".format(global_maxP))



   return( (list_task_time, list_task_iter, list_task_finished) )
         
      

if __name__ == '__main__':
   parser = argparse.ArgumentParser()
   parser.add_argument("log", help='Log file to scan', type=str, nargs='+')
   parser.add_argument("--tasks", "-t", help='Tasks file', type=str, default="tasksend.txt")
   parser.add_argument("--restart", "-r", help='Create restart file', type=str, default="restart.txt")
   parser.add_argument("--save-log", "-o", help='Save short log file (2 lines per task)', type=str)
   args = parser.parse_args()
   tasks = scan_tasks(args.tasks)
   print("Number of tasks = {}".format(len(tasks)))
   stat = scan_log(args, tasks)
