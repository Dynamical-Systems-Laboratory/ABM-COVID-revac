#include "../../../include/abm.h"
#include <chrono>

/***************************************************** 
 *
 * ABM run of COVID-19 SEIR in New Rochelle, NY 
 *
 ******************************************************/

int main()
{
	// Time in days, space in km
	double dt = 0.25;
	// Max number of steps to simulate
	int tmax = 720;	
	// Print agent info this many steps
	int dt_out_agents = 100; 
	// Number of initially infected
	int inf0 = 1;
	// Number of agents in different stages of COVID-19
	int N_active = 36, N_vac = 48067;
	// Have agents vaccinated already
	bool vaccinate = true;
	// Don't vaccinate in the setup phase to have agents 
	// vaccinated with a time offset
	bool dont_vac = true; 

	// File with all the input files names
	std::string fin("input_data/input_files_all_vac_reopen.txt");

	// Output file names
	// Active at the current step - all, detected and not 
	std::ofstream ftot_inf_cur("output/infected_with_time.txt");
	// Cumulative
	std::ofstream ftot_inf("output/total_infected.txt");
	std::ofstream ftot_dead("output/dead_with_time.txt");

	// This initializes the core of the model
	ABM abm(dt);
	abm.simulation_setup(fin, inf0);

	// Initialization for vaccination/reopening studies
	abm.initialize_vac_and_reopening(dont_vac);
	// Create a COVID-19 population with previously vaccinated at random times
	abm.initialize_active_cases(N_active, vaccinate, N_vac);	

	// Simulation
	std::vector<int> active_count(tmax+1);
	std::vector<int> infected_count(tmax+1);
	std::vector<int> total_dead(tmax+1);

	// For time measurement
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	for (int ti = 0; ti<=tmax; ++ti){
		// Save agent information
		/*if (ti%dt_out_agents == 0){
			std::string fname = "output/agents_t_" + std::to_string(ti) + ".txt";
			abm.print_agents(fname);
		}*/
		// Collect data
		active_count.at(ti) = abm.get_num_infected();
		infected_count.at(ti) = abm.get_total_infected();
		total_dead.at(ti) = abm.get_total_dead();
		
		// Propagate 
		abm.transmit_with_vac();
	}

	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count() << "[ms]" << std::endl;
	std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() << "[s]" << std::endl;

	// Totals
	// Infection
	std::copy(active_count.begin(), active_count.end(), std::ostream_iterator<int>(ftot_inf_cur, " "));
	std::copy(infected_count.begin(), infected_count.end(), std::ostream_iterator<int>(ftot_inf, " "));
	std::copy(total_dead.begin(), total_dead.end(), std::ostream_iterator<int>(ftot_dead, " "));
	
	// Print total values
	std::cout << "Total number of infected agents: " << abm.get_total_infected() << "\n"
			  << "Total number of casualities: " << abm.get_total_dead() << "\n"
			  << "Total number of recovered agents: " << abm.get_total_recovered() << "\n";
}
