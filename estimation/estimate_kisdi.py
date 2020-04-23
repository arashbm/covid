import sys
import os
import csv
import tempfile
import subprocess
import itertools
import shutil

import numpy as np
import pandas as pd
from scipy import stats
import elfi
import matplotlib.pyplot as plt

def simulate(
        population_filename, area_names, days,
        param_keys, ic_keys, *parameters, random_state=None):

    p = {k: v for k, v in zip(param_keys, parameters)}
    if not random_state:
        random_state = np.random
    seed = random_state.randint(0, 2**16-1)

    prefixdir = None
    if 'XDG_RUNTIME_DIR' in os.environ:
        prefixdir = os.environ['XDG_RUNTIME_DIR']
    with tempfile.TemporaryDirectory(dir=prefixdir) as tmpdir:
        initial_condition_filename = \
            os.path.join(tmpdir, 'initial_condition.csv')
        ics = [(*k, v) for k, v
                in zip(ic_keys, parameters[len(param_keys):])]

        ic_columns = ['Area', 'Compartment', 'Age Group', 'Count']
        ics_df = pd.DataFrame(ics,
                columns=ic_columns).\
                        set_index(ic_columns[:3]).unstack(level=[-2, -1])
        ics_df.columns = [f"{comp} {age}"
                for _, comp, age in ics_df.columns.values]
        ics_df.to_csv(initial_condition_filename, header=True)


        model_config_filename = \
                os.path.join(tmpdir, 'model-parameters.csv')

        with open(model_config_filename, 'w', newline='') as model_file:
            writer = csv.DictWriter(model_file, fieldnames=['Name', 'Value'])
            writer.writeheader()

            for age1 in ['y', 'm', 'o']:
                for age2, v in zip(['y', 'm', 'o'], p.pop(f'contact_{age1}')):
                    writer.writerow({
                        'Name': f'c_{age1}{age2}',
                        'Value': v})

            for k, v in p.items():
                writer.writerow({'Name': k, 'Value': v})


        p = subprocess.run([
                "../municipalities",
                "-t", str(days),
                "-i", initial_condition_filename,
                "-c", model_config_filename,
                "-s", str(seed),
                "-p", population_filename],
                    capture_output=True)

        if p.returncode != 0:
            print(p)
            print(p.stderr.decode("utf-8"))
            debug_dir = f"/tmp/debug{seed}"
            os.makedirs(debug_dir, exist_ok=True)
            shutil.copyfile(initial_condition_filename,
                    os.path.join(debug_dir, 'initial.csv'))
            shutil.copyfile(model_config_filename,
                    os.path.join(debug_dir, 'model.csv'))
            print("files copied to", debug_dir)
        p.check_returncode()

        observations = []
        text = p.stdout.decode('utf-8').split('\n')
        for row in csv.DictReader(text, delimiter=' '):
            observations.append((row['dead'], row['hospitalized']))

        return np.array(observations[:days])

def sim_node(population_filename, observed):
    model = elfi.new_model()
    areas = set()
    with open(population_filename, newline='') as f:
        for row in csv.DictReader(f):
            areas.add(row['Area'])

    univariate_params = {
            'beta_presymptomatic':  stats.uniform(0, 1),
            'beta_asymptomatic':    stats.uniform(0, 1),
            'beta_infected':        stats.uniform(0, 1),
            'pi':                   stats.beta(4, 6),
            'kappa':                stats.beta(2, 8),
            'reciprocal_eta':       stats.gamma(2.34,  5.0),
            'reciprocal_alpha':     stats.gamma(2.,  5.0),
            'reciprocal_theta':     stats.gamma(8.0,  5.0),
            'reciprocal_nu':        stats.gamma(2.86, 5.0),
            'reciprocal_rho':       stats.gamma(5.0,  5.0),
            'reciprocal_chi':       stats.gamma(10.0, 5.0),
            'reciprocal_delta':     stats.gamma(7.0, 5.0),
    }

    multivar_params = {
            'contact_y': stats.dirichlet([1, 1, 1]),
            'contact_m': stats.dirichlet([1, 1, 1]),
            'contact_o': stats.dirichlet([1, 1, 1]),
    }


    parameters = {}
    for k, v in itertools.chain(
            univariate_params.items(),
            multivar_params.items()):
        parameters[k] = elfi.Prior(v, name=k, model=model)

    initial_condition_parameters = {}
    for area in list(areas):
        for compartment in ['Exposed', 'Presymptomatic',
                'Asymptomatic', 'Infected']:
            for age_group in ['Young', 'Adults', 'Elderly']:
                key = (area, compartment, age_group)
                initial_condition_parameters[key] = \
                        elfi.Constant(1, name=' '.join(key),
                                model=model)
                        # elfi.Prior(stats.uniform(0, 1000), name=' '.join(key),
                        #         model=model)


    sim_fun = elfi.tools.vectorize(simulate, constants=[0, 1, 2, 3, 4])
    sim = elfi.Simulator(sim_fun,
            population_filename, list(areas), observed.shape[0],
            list(parameters), list(initial_condition_parameters),
            *parameters.values(), *initial_condition_parameters.values(),
            observed = deaths)

    return model, sim,\
            univariate_params, multivar_params, initial_condition_parameters

if __name__ == "__main__":
    elfi.set_client('multiprocessing')

    if len(sys.argv) != 2:
        raise "no population file was given"

    # (cumulative) deaths every day from 1st of march
    deaths = [0]*20 + [1, 1, 1, 1, 3, 4, 7, 9, 11, 13, 17, 17, 19, 20, 25, 27,
            27, 34, 40, 42, 47, 49, 56, 59, 64, 72, 75]

    # In ICU
    hospitalized = [np.nan]*24 + [22, 24, 32, 31, 41, 49, 56, 62, 65, 72, 73,
            76, 81, 83, 82, 82, 81, 80, 77, 74, 75, 75, 76]

    # total hospitalized
    # hospitalized = [np.nan]*24 + [82, 96, 108, 112, 134, 143, 137, 159, 160,
    #         180, 187, 209, 228, 231, 239, 244, 236, 235, 235, 230, 232, 226,
    #         215]

    observed = np.array(list(zip(deaths, hospitalized)))

    population_filename = sys.argv[1]
    model, sim, univariate_params, multivar_params, \
            initial_condition_parameters = sim_node(
                    population_filename, observed)

    dist = elfi.Distance('euclidean', sim, model=model)
    log_d = elfi.Operation(np.log, dist)

    sampler = elfi.Rejection(dist, batch_size=100, seed=42)
    results = sampler.sample(1000, quantile=0.001)
    results.summary()

    for k, v in results.sample_means.items():
        print(f"{k}: {v}")

    os.makedirs('../figures', exist_ok=True)
    for p in univariate_params:
        if p in results.samples:
            fig, ax = plt.subplots()
            ax.set_ylabel(p)
            ax.hist(results.samples[p], bins=25, density=True)
            x = np.linspace(
                    np.min(results.samples[p]),
                    np.max(results.samples[p]), 100)
            ax.plot(x, univariate_params[p].pdf(x))
            fig.savefig(f"../figures/parameters-{p}.pdf")
            plt.close(fig=fig)

    for p in initial_condition_parameters:
        name = ' '.join(p)
        if name in results.samples:
            file_name = '-'.join(p)
            fig, ax = plt.subplots()
            ax.set_ylabel(name)
            ax.hist(results.samples[name], bins=25, density=True)
            fig.savefig(f"../figures/parameters-ic-{file_name}.pdf")
            plt.close(fig=fig)
