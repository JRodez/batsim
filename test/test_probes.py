#!/usr/bin/env python3
'''Simple tests.

These tests run batsim to test if probes are correctly implemented.
'''
from helper import *

def probe(platform, workload, algorithm):
    test_name = f'probes-{algorithm.name}-{platform.name}-{workload.name}'
    output_dir, robin_filename, _ = init_instance(test_name)

    # if algorithm.sched_implem != 'pybatsim': raise Exception('This test only supports pybatsim for now')

    batcmd = gen_batsim_cmd(platform.filename, workload.filename, output_dir, "")
    instance = RobinInstance(output_dir=output_dir,
        batcmd=batcmd+' --energy',
        schedcmd=f"pybatsim testProbesOneShot",
        simulation_timeout=30, ready_timeout=5,
        success_timeout=10, failure_timeout=0
    )

    instance.to_file(robin_filename)
    ret = run_robin(robin_filename)
    if ret.returncode != 0: raise Exception(f'Bad robin return code ({ret.returncode})')

def test_probe1(energy_platform, small_workload, probe_algorithm):
    probe(energy_platform, small_workload, probe_algorithm)

