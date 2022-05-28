import psutil

cpu_percent_cores = psutil.cpu_percent(interval=2, percpu=True)
avg = sum(cpu_percent_cores)/len(cpu_percent_cores)
cpu_percent_total_str = ('%.2f' % avg) + '%'
cpu_percent_cores_str = [('%.2f' % x) + '%' for x in cpu_percent_cores]
print('Total: {}'.format(cpu_percent_total_str))
print('Individual CPUs: {}'.format('  '.join(cpu_percent_cores_str)))