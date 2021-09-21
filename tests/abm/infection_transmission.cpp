#include "abm_tests.h"

/***************************************************** 
 *
 * Test suite for infection related computations
 *
******************************************************/

// Tests
bool abm_contributions_test();
bool abm_leisure_dist_test();
bool abm_events_test();
bool abm_time_dependent_testing();
bool abm_vaccination();
bool abm_vac_reopening();
bool abm_vac_reopening_seeded();
bool abm_vac_reopening_seeded_with_vaccinated();

// Supporting functions
bool abm_vaccination_random();
bool abm_vaccination_group();
ABM create_abm(const double dt, int i0);
bool vaccinated_flag_check(const Agent& agent, const int);

int main()
{
	test_pass(abm_leisure_dist_test(), "Assigning leisure locations");
	test_pass(abm_events_test(), "Testing and lockdown events");
	test_pass(abm_time_dependent_testing(), "Time dependent testing");
  	test_pass(abm_vaccination(), "Vaccination");
	test_pass(abm_vac_reopening(), "Reopening and vaccination studies");
	test_pass(abm_vac_reopening_seeded(), "Initializing with active COVID-19 cases");
	test_pass(abm_vac_reopening_seeded_with_vaccinated(), "Initializing with active and vaccinated cases");
}

bool abm_leisure_dist_test()
{
	// Agents 
	std::string fin("test_data/NR_agents.txt");
	// Places
	std::string hfile("test_data/NR_households.txt");
	std::string sfile("test_data/NR_schools.txt");
	std::string wfile("test_data/NR_workplaces.txt");
	std::string hspfile("test_data/NR_hospitals.txt");
	std::string rh_file("test_data/NR_retirement_homes.txt");
	std::string cp_file("test_data/NR_carpool.txt");
	std::string pt_file("test_data/NR_public.txt");
	std::string ls_file("test_data/NR_leisure.txt");
	std::string fmob_out("test_data/mobility_probs_out.txt");

	// Model parameters
	double dt = 0.25;
	int tmax = 1;
	int N_covid = 15000;

	// File with infection parameters
	std::string pfname("test_data/vac_reopen_infection_parameters.txt");
	// Files with age-dependent distributions
	std::string dexp_name("test_data/age_dist_exposed_never_sy.txt");
	std::string dh_name("test_data/age_dist_hospitalization.txt");
	std::string dhicu_name("test_data/age_dist_hosp_ICU.txt");
	std::string dmort_name("test_data/age_dist_mortality.txt");
	// Map for abm loading of distrinutions
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", dexp_name}, {"hospitalization", dh_name}, {"ICU", dhicu_name}, {"mortality", dmort_name} };	
	// File with time dependent testing parameters
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	// First the places
	abm.create_households(hfile);
	abm.create_schools(sfile);
	abm.create_workplaces(wfile);
	abm.create_hospitals(hspfile);
	abm.create_retirement_homes(rh_file);
	abm.create_carpools(cp_file);
	abm.create_public_transit(pt_file);
	abm.create_leisure_locations(ls_file);
	abm.initialize_mobility();

	//abm.print_mobility_probabilities(fmob_out);

	// Then the agents
	abm.create_agents(fin);

	// Seed and vaccinate
	abm.initialize_vac_and_reopening();
	abm.initialize_active_cases(N_covid);

	// Make some agents more mobile
	std::vector<Agent>& agents = abm.vector_of_agents();
	Infection infection = abm.get_copied_infection_object();
	for (auto& agent : agents) {
		if (agent.vaccinated() && infection.get_uniform() < 0.75) {
			agent.set_more_active(true);
		}
	}

 	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();	
	// Simulation
	int n_lhs = 0, n_ltot = 0, n_tot = 0, n_same = 0, n_more = 0;
	for (int ti = 0; ti<=tmax; ++ti) {
		const std::vector<Agent> agents_0 = abm.get_copied_vector_of_agents();
		abm.transmit_infection();
		const std::vector<Agent> agents_F = abm.get_copied_vector_of_agents();
		const std::vector<Household>& households = abm.get_vector_of_households();	
		const std::vector<Leisure>& leisure_locations = abm.get_vector_of_leisure_locations();

		for (int i=0; i < agents_0.size(); ++i) {
			int L_ID0 = agents_0.at(i).get_leisure_ID(); 
			int L_IDF = agents_F.at(i).get_leisure_ID();
			std::string L_type0 = agents_0.at(i).get_leisure_type(); 
			std::string L_typeF = agents_F.at(i).get_leisure_type();
			++n_tot;

			if (L_ID0 > 0) {
				int aID = agents_0.at(i).get_ID();
				if ((L_type0 == L_typeF) && (L_ID0 == L_IDF)) {
					// This is possible, but it shouldn't happen too often
					++n_same;
				} else {
					// Should be properly removed
					if (L_type0 == "household") {
						std::vector<int> agent_IDs = households.at(L_ID0-1).get_agent_IDs();
						if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
										!= agent_IDs.end()) {
							std::cerr << "Agent still registered in a household as a leisure location" << std::endl;
							return false;	
						}
					} else if (L_type0 == "public") {
						std::vector<int> agent_IDs = leisure_locations.at(L_ID0-1).get_agent_IDs();
						if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
										!= agent_IDs.end()) {
							std::cerr << "Agent still registered in the previous leisure location" << std::endl;
						  	return false;	
						}
					}
				}
			}
			if (L_IDF > 0) {
				int aID = agents_F.at(i).get_ID();
				++n_ltot;
				// Check for agents that are more mobile because of the vaccination
				if (agents_F.at(i).more_active()) {
					++n_more;
				}
				// Should be added 
				if (L_typeF == "household") {
					++n_lhs;
					std::vector<int> agent_IDs = households.at(L_IDF-1).get_agent_IDs();
					if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
									== agent_IDs.end()) {
						std::cerr << "Agent not registered in a household as a leisure location" << std::endl;
						return false;
					}	
				} else if (L_typeF == "public" && !leisure_locations.at(L_IDF-1).outside_town()) {
					std::vector<int> agent_IDs = leisure_locations.at(L_IDF-1).get_agent_IDs();
					if ((std::find(agent_IDs.begin(), agent_IDs.end(), aID)) 
									== agent_IDs.end()) {
						std::cerr << "Agent not registered in the leisure location" << std::endl;
						return false;	
					}
				}
			}
		}
	}
	double fr_lhs = static_cast<double>(n_lhs)/static_cast<double>(n_ltot);
	std::cout << "Distribution of leisure locations - "
			  << "fraction of households: " << fr_lhs << std::endl;
	
	double fr_same = static_cast<double>(n_same)/static_cast<double>(n_ltot);
	std::cout << "Distribution of leisure locations - "
			  << "fraction of repeated locations two steps in a row: " << fr_same << std::endl;
	
	double fr_leisure = static_cast<double>(n_ltot)/static_cast<double>(n_tot);
	if (!float_equality<double>(fr_leisure, infection_parameters.at("leisure - fraction"), 0.1)) {
		std::cerr << "Wrong fraction of agents going to leisure locations" << std::endl;
		return false;	
	}
	double fr_more_leisure = static_cast<double>(n_more)/static_cast<double>(n_tot);
	if (fr_more_leisure <= 0) {
		std::cerr << "Fraction of more mobile agents going to leisure" 
				  << " should be larger than zero" << std::endl;
		return false;	
	}
	return true;
}

bool abm_events_test()
{
	// Agents 
	std::string fin("test_data/NR_agents.txt");
	// Places
	std::string hfile("test_data/NR_households.txt");
	std::string sfile("test_data/NR_schools.txt");
	std::string wfile("test_data/NR_workplaces.txt");
	std::string hspfile("test_data/NR_hospitals.txt");
	std::string rh_file("test_data/NR_retirement_homes.txt");
	std::string cp_file("test_data/NR_carpool.txt");
	std::string pt_file("test_data/NR_public.txt");
	std::string ls_file("test_data/NR_leisure.txt");
	std::string fmob_out("test_data/mobility_probs_out.txt");
	
	// Model parameters
	double dt = 0.25;
	int tmax = 200;

	// File with infection parameters
	std::string pfname("test_data/infection_parameters.txt");
	// Files with age-dependent distributions
	std::string dexp_name("test_data/age_dist_exposed_never_sy.txt");
	std::string dh_name("test_data/age_dist_hospitalization.txt");
	std::string dhicu_name("test_data/age_dist_hosp_ICU.txt");
	std::string dmort_name("test_data/age_dist_mortality.txt");
	// Map for abm loading of distrinutions
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", dexp_name}, {"hospitalization", dh_name}, {"ICU", dhicu_name}, {"mortality", dmort_name} };	
	// File with time dependent testing parameters
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	// First the places
	abm.create_households(hfile);
	abm.create_schools(sfile);
	abm.create_workplaces(wfile);
	abm.create_hospitals(hspfile);
	abm.create_retirement_homes(rh_file);
	abm.create_carpools(cp_file);
	abm.create_public_transit(pt_file);
	abm.create_leisure_locations(ls_file);
	abm.initialize_mobility();

	// Then the agents
	abm.create_agents(fin);

	// Contains event times and properties
	std::map<std::string, double>& infection_parameters = abm.get_infection_parameters(); 
	double tol = 1e-3;
	double time = 0.0;
	const double leisure_fraction =  infection_parameters.at("leisure - fraction");

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){
		// Propagate
		time = abm.get_time(); 
		abm.transmit_infection();

		// School closure - school rate is 0.0
		if (float_equality<double>(time, infection_parameters.at("school closure"), tol)){
			const std::vector<School>& schools =  abm.get_vector_of_schools();
			for (const auto& school : schools){
				if (!float_equality<double>(school.get_transmission_rate(), 0.0, tol)){
					std::cerr << "Error in school closure - student transmission rate not 0.0" << std::endl; 
					return false;	
				}
				if (!float_equality<double>(school.get_employee_transmission_rate(), 0.0, tol)){
					std::cerr << "Error in school closure - employee transmission rate not 0.0" << std::endl; 
					return false;	
				}
			}
		}

		// Non-essential workplace closure - reduced rate
		if (float_equality<double>(time, infection_parameters.at("lockdown"), tol)){
			double new_tr_rate = infection_parameters.at("workplace transmission rate")*
									infection_parameters.at("fraction of ld businesses");
			double new_tr_rate_outside = infection_parameters.at("out-of-town workplace transmission")*
									infection_parameters.at("fraction of ld businesses");
			const std::vector<Workplace>& workplaces =  abm.get_vector_of_workplaces();
			for (const auto& workplace : workplaces){
				if (!workplace.outside_town()) {
					if (!float_equality<double>(workplace.get_absenteeism_correction(), infection_parameters.at("lockdown absenteeism"), tol)){
						std::cerr << "Error in lockdown - wrong workplace absenteeism correction" << std::endl; 
						return false;	
					}
					if (!float_equality<double>(workplace.get_transmission_rate(), new_tr_rate, tol)){
						std::cerr << "Error in lockdown - wrong workplace transmission rate" << std::endl; 
						return false;	
					}
				} else {
/*					if (!float_equality<double>(workplace.get_outside_lambda(), new_tr_rate_outside, tol)){
						std::cerr << "Error in lockdown - wrong out of town workplace transmission rate" << std::endl; 
						return false;	
					}*/
				}
			}
			const std::vector<Leisure>& leisure_locations = abm.get_vector_of_leisure_locations();
			new_tr_rate = infection_parameters.at("leisure locations transmission rate")*
							infection_parameters.at("fraction of ld businesses");
			new_tr_rate_outside = infection_parameters.at("out-of-town leisure transmission")*
									infection_parameters.at("fraction of ld businesses");
			double new_l_frac = leisure_fraction * infection_parameters.at("fraction of ld businesses"); 
			for (const auto& lloc : leisure_locations) {
				if (!lloc.outside_town()){
					if (!float_equality<double>(lloc.get_transmission_rate(), new_tr_rate, tol)){
						std::cerr << "Error in lockdown - wrong leisure location transmission rate" << std::endl; 
						return false;	
					}
				}else{
					if (!float_equality<double>(lloc.get_outside_lambda(), new_tr_rate_outside, tol)){
						std::cerr << "Error in lockdown - wrong out of town leisure transmission rate" << std::endl; 
						return false;	
					}
				}
			}
			if (!float_equality<double>(infection_parameters.at("leisure - fraction"), new_l_frac, tol)){
				std::cerr << "Error in lockdown - wrong leisure fraction " << std::endl; 
				return false;	
			}
			// Carpools
			const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
			new_tr_rate = infection_parameters.at("carpool transmission rate")
							*infection_parameters.at("fraction of ld businesses");
			for (const auto& cpl : carpools) {
				if (!float_equality<double>(cpl.get_transmission_rate(), new_tr_rate, tol)){
					std::cerr << "Error in lockdown - wrong carpool transmission rate" << std::endl; 
					return false;	
				}
			}
			// Public transit
			new_tr_rate = infection_parameters.at("public transit beta0") 
						+ infection_parameters.at("public transit beta full")
						*infection_parameters.at("public transit current capacity")*infection_parameters.at("fraction of ld businesses");
			const std::vector<Transit>& public_transit = abm.get_vector_of_public_transit();
			for (auto& pt  : public_transit) {
				if (!float_equality<double>(pt.get_transmission_rate(), new_tr_rate, tol)){
					std::cerr << "Error in lockdown - wrong public transit transmission rate" << std::endl; 
					return false;	
				}
			}
		}

		// Reopening phase 1 - more businesses open
		if (float_equality<double>(time, infection_parameters.at("reopening phase 1"), tol)){
			double new_tr_rate = infection_parameters.at("workplace transmission rate")*
									infection_parameters.at("fraction of phase 1 businesses");
			double new_tr_rate_outside = infection_parameters.at("out-of-town workplace transmission")*
									infection_parameters.at("fraction of phase 1 businesses");
			const std::vector<Workplace>& workplaces =  abm.get_vector_of_workplaces();
			for (const auto& workplace : workplaces){
				if (!workplace.outside_town()) {
					if (!float_equality<double>(workplace.get_absenteeism_correction(), infection_parameters.at("lockdown absenteeism"), tol)){
						std::cerr << "Error in reopening phase 1 - wrong workplace absenteeism correction" << std::endl; 
						return false;	
					}
					if (!float_equality<double>(workplace.get_transmission_rate(), new_tr_rate, tol)){
						std::cerr << "Error in reopening phase 1 - wrong workplace transmission rate" << std::endl; 
						return false;	
					}
				} else {
					// To be updated when this part of the code changes to new uproach
/*					if (!float_equality<double>(workplace.get_outside_lambda(), new_tr_rate_outside, tol)){
						std::cerr << "Error in reopening phase 1 - wrong out of town workplace transmission rate" << std::endl; 
						return false;	
					}*/
				}
			}
			const std::vector<Leisure>& leisure_locations = abm.get_vector_of_leisure_locations();
			new_tr_rate = infection_parameters.at("leisure locations transmission rate")*
									infection_parameters.at("fraction of phase 1 businesses");
			new_tr_rate_outside = infection_parameters.at("out-of-town leisure transmission")*
									infection_parameters.at("fraction of phase 1 businesses");
			for (const auto& lloc : leisure_locations) {
				if (!lloc.outside_town()){
					if (!float_equality<double>(lloc.get_transmission_rate(), new_tr_rate, tol)){
						std::cerr << "Error in reopening phase 1 - wrong leisure location transmission rate" << std::endl; 
						return false;	
					}
				}else{
					if (!float_equality<double>(lloc.get_outside_lambda(), new_tr_rate_outside, tol)){
						std::cerr << "Error in reopening phase 1 - wrong out of town leisure location transmission rate" << std::endl; 
						return false;	
					}
				}
			}
			double new_l_frac = leisure_fraction*infection_parameters.at("fraction of phase 1 businesses");
			if (!float_equality<double>(infection_parameters.at("leisure - fraction"), new_l_frac, tol)){
				std::cerr << "Error in phase 1 - wrong leisure fraction" << std::endl; 
				return false;	
			}
			// Carpools
			const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
			new_tr_rate = infection_parameters.at("carpool transmission rate")
							*infection_parameters.at("fraction of phase 1 businesses");
			for (const auto& cpl : carpools) {
				if (!float_equality<double>(cpl.get_transmission_rate(), new_tr_rate, tol)){
					std::cerr << "Error in reopening phase 1 - wrong carpool transmission rate" << std::endl; 
					return false;	
				}
			}
			// Public transit
			new_tr_rate = infection_parameters.at("public transit beta0") 
					+ infection_parameters.at("public transit beta full")
					*infection_parameters.at("public transit current capacity")*infection_parameters.at("fraction of phase 1 businesses");
			const std::vector<Transit>& public_transit = abm.get_vector_of_public_transit();
			for (auto& pt  : public_transit) {
				if (!float_equality<double>(pt.get_transmission_rate(), new_tr_rate, tol)){
					std::cerr << "Error in reopening phase 1 - wrong public transit transmission rate" << std::endl; 
					return false;	
				}
			}
		}

		// Reopening phase 2 - even more businesses open
		if (float_equality<double>(time, infection_parameters.at("reopening phase 2"), tol)){
			double new_tr_rate = infection_parameters.at("workplace transmission rate")*
									infection_parameters.at("fraction of phase 2 businesses");
			double new_tr_rate_outside = infection_parameters.at("out-of-town workplace transmission")*
									infection_parameters.at("fraction of phase 2 businesses");
			const std::vector<Workplace>& workplaces =  abm.get_vector_of_workplaces();
			for (const auto& workplace : workplaces){
				if (!workplace.outside_town()) {
					if (!float_equality<double>(workplace.get_absenteeism_correction(), infection_parameters.at("lockdown absenteeism"), tol)){
						std::cerr << "Error in reopening phase 2 - wrong workplace absenteeism correction" << std::endl; 
						return false;	
					}
					if (!float_equality<double>(workplace.get_transmission_rate(), new_tr_rate, tol)){
						std::cerr << "Error in reopening phase 2 - wrong workplace transmission rate" << std::endl; 
						return false;	
					}
				} else {
					/*if (!float_equality<double>(workplace.get_outside_lambda(), new_tr_rate_outside, tol)){
						std::cerr << "Error in reopening phase 2 - wrong out of town workplace transmission rate" << std::endl; 
						return false;	
					}*/
				}
			}
			const std::vector<Leisure>& leisure_locations = abm.get_vector_of_leisure_locations();
			new_tr_rate = infection_parameters.at("leisure locations transmission rate")*
									infection_parameters.at("fraction of phase 2 businesses");
			new_tr_rate_outside = infection_parameters.at("out-of-town leisure transmission")*
									infection_parameters.at("fraction of phase 2 businesses");
			for (const auto& lloc : leisure_locations) {
				if (!lloc.outside_town()){
					if (!float_equality<double>(lloc.get_transmission_rate(), new_tr_rate, tol)){
						std::cerr << "Error in reopening phase 2 - wrong leisure location transmission rate" << std::endl; 
						return false;	
					}
				}else{
					if (!float_equality<double>(lloc.get_outside_lambda(), new_tr_rate_outside, tol)){
						std::cerr << "Error in reopening phase 2 - wrong out of town leisure location transmission rate" << std::endl; 
						return false;	
					}
				}
			}
			double new_l_frac = leisure_fraction*infection_parameters.at("fraction of phase 2 businesses");
			if (!float_equality<double>(infection_parameters.at("leisure - fraction"), new_l_frac, tol)){
				std::cerr << "Error in phase 2 - wrong leisure fraction" << std::endl; 
				return false;	
			}
			// Carpools
			const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
			new_tr_rate = infection_parameters.at("carpool transmission rate")
							*infection_parameters.at("fraction of phase 2 businesses");
			for (const auto& cpl : carpools) {
				if (!float_equality<double>(cpl.get_transmission_rate(), new_tr_rate, tol)){
					std::cerr << "Error in reopening phase 2 - wrong carpool transmission rate" << std::endl; 
					return false;	
				}
			}
			// Public transit
			new_tr_rate = infection_parameters.at("public transit beta0") 
					+ infection_parameters.at("public transit beta full")
					*infection_parameters.at("public transit current capacity")*infection_parameters.at("fraction of phase 2 businesses");
			const std::vector<Transit>& public_transit = abm.get_vector_of_public_transit();
			for (auto& pt  : public_transit) {
				if (!float_equality<double>(pt.get_transmission_rate(), new_tr_rate, tol)){
					std::cerr << "Error in reopening phase 2 - wrong public transit transmission rate" << std::endl; 
					return false;	
				}
			}
		}

		// Reopening phase 3 - restaurants open
		if (float_equality<double>(time, infection_parameters.at("reopening phase 3"), tol)){
			double new_tr_rate = infection_parameters.at("workplace transmission rate")*
									infection_parameters.at("fraction of phase 3 businesses");
			double new_tr_rate_outside = infection_parameters.at("out-of-town workplace transmission")*
									infection_parameters.at("fraction of phase 3 businesses");
			const std::vector<Workplace>& workplaces =  abm.get_vector_of_workplaces();
			for (const auto& workplace : workplaces){
				if (!workplace.outside_town()) {
					if (!float_equality<double>(workplace.get_absenteeism_correction(), infection_parameters.at("lockdown absenteeism"), tol)){
						std::cerr << "Error in reopening phase 3 - wrong workplace absenteeism correction" << std::endl; 
						return false;	
					}
					if (!float_equality<double>(workplace.get_transmission_rate(), new_tr_rate, tol)){
						std::cerr << "Error in reopening phase 3 - wrong workplace transmission rate" << std::endl; 
						return false;	
					}
				} else {
					/*if (!float_equality<double>(workplace.get_outside_lambda(), new_tr_rate_outside, tol)){
						std::cerr << "Error in reopening phase 3 - wrong out of town workplace transmission rate" << std::endl; 
						return false;	
					}*/
				}
			}
			const std::vector<Leisure>& leisure_locations = abm.get_vector_of_leisure_locations();
			new_tr_rate = infection_parameters.at("leisure locations transmission rate")*
									infection_parameters.at("fraction of phase 3 businesses");
			new_tr_rate_outside = infection_parameters.at("out-of-town leisure transmission")*
									infection_parameters.at("fraction of phase 3 businesses");
			for (const auto& lloc : leisure_locations) {
				if (!lloc.outside_town()){
					if (!float_equality<double>(lloc.get_transmission_rate(), new_tr_rate, tol)){
						std::cerr << "Error in reopening phase 3 - wrong leisure location transmission rate" << std::endl; 
						return false;	
					}
				}else{
					if (!float_equality<double>(lloc.get_outside_lambda(), new_tr_rate_outside, tol)){
						std::cerr << "Error in reopening phase 3 - wrong out of town leisure location transmission rate" << std::endl; 
						return false;	
					}
				}
			}
			double new_l_frac = leisure_fraction*infection_parameters.at("fraction of phase 3 businesses");
			if (!float_equality<double>(infection_parameters.at("leisure - fraction"), new_l_frac, tol)){
				std::cerr << "Error in phase 3 - wrong leisure fraction" << std::endl; 
				return false;	
			}
			// Carpools
			const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
			new_tr_rate = infection_parameters.at("carpool transmission rate")
							*infection_parameters.at("fraction of phase 3 businesses");
			for (const auto& cpl : carpools) {
				if (!float_equality<double>(cpl.get_transmission_rate(), new_tr_rate, tol)){
					std::cerr << "Error in reopening phase 3 - wrong carpool transmission rate" << std::endl; 
					return false;	
				}
			}
			// Public transit
			new_tr_rate = infection_parameters.at("public transit beta0") 
					+ infection_parameters.at("public transit beta full")
					*infection_parameters.at("public transit current capacity")*infection_parameters.at("fraction of phase 3 businesses");
			const std::vector<Transit>& public_transit = abm.get_vector_of_public_transit();
			for (auto& pt  : public_transit) {
				if (!float_equality<double>(pt.get_transmission_rate(), new_tr_rate, tol)){
					std::cerr << "Error in reopening phase 3 - wrong public transit transmission rate" << std::endl; 
					return false;	
				}
			}
		}

	}
	return true;
}

bool abm_time_dependent_testing()
{
	// Agents 
	std::string fin("test_data/NR_agents.txt");
	// Places
	std::string hfile("test_data/NR_households.txt");
	std::string sfile("test_data/NR_schools.txt");
	std::string wfile("test_data/NR_workplaces.txt");
	std::string hspfile("test_data/NR_hospitals.txt");
	std::string rh_file("test_data/NR_retirement_homes.txt");
	std::string cp_file("test_data/NR_carpool.txt");
	std::string pt_file("test_data/NR_public.txt");
	std::string ls_file("test_data/NR_leisure.txt");
	
	// Model parameters
	double dt = 0.25;
	int tmax = 300;

	// File with infection parameters
	std::string pfname("test_data/infection_parameters.txt");
	// Files with age-dependent distributions
	std::string dexp_name("test_data/age_dist_exposed_never_sy.txt");
	std::string dh_name("test_data/age_dist_hospitalization.txt");
	std::string dhicu_name("test_data/age_dist_hosp_ICU.txt");
	std::string dmort_name("test_data/age_dist_mortality.txt");
	// Map for abm loading of distrinutions
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", dexp_name}, {"hospitalization", dh_name}, {"ICU", dhicu_name}, {"mortality", dmort_name} };	
	// File with time dependent testing parameters	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	// First the places
	abm.create_households(hfile);
	abm.create_schools(sfile);
	abm.create_workplaces(wfile);
	abm.create_hospitals(hspfile);
	abm.create_retirement_homes(rh_file);
	abm.create_carpools(cp_file);
	abm.create_public_transit(pt_file);
	abm.create_leisure_locations(ls_file);

	abm.initialize_mobility();

	abm.create_agents(fin);

	// Contains event times and properties
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();
   	const std::vector<std::vector<double>> exp_values = {{27.0, 0.1, 0.5}, {60.0, 1.0, 0.4}};	
	double tol = 1e-3;
	double time = 0.0;
	double flu_prob = 0.0;
	int exp_num_changes = exp_values.size();
	int change_count = 0;

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){
		// Propagate
		time = abm.get_time(); 
		abm.transmit_infection();

		// No testing, no flu - check for all agents 
		if (time < infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			for (const auto& agent : agents){
				if ((agent.tested() == true) || (agent.symptomatic_non_covid() == true)){
					std::cerr << "Agents tested before testing is supposed to start" << std::endl;
					return false;
				}
			}
		}
		// Testing starts with initial testing values
		if (float_equality<double>(time, infection_parameters.at("start testing"), tol)){
			const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();
			Testing testing = abm.get_testing_object();
			flu_prob = (infection_parameters.at("fraction false positive") + infection_parameters.at("negative tests fraction"))
							*infection_parameters.at("fraction to get tested");
			if (!float_equality<double>(testing.get_sy_tested_prob(), 
									infection_parameters.at("fraction to get tested"), 1e-5)||
				!float_equality<double>(testing.get_exp_tested_prob(), 
									infection_parameters.at("exposed fraction to get tested"), 1e-5) || 
				!float_equality<double>(testing.get_prob_flu_tested(), flu_prob, 1e-5)){
				std::cerr << "Wrong initial testing values" << std::endl;
				return false;
			}	
		}
		// Now check each switch 
		for (const auto& tch : exp_values){
			if (float_equality<double>(time, tch.at(0), tol)){
				++change_count;
				const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();
				Testing testing = abm.get_testing_object();
				flu_prob = (infection_parameters.at("fraction false positive") 
								+ infection_parameters.at("negative tests fraction"))*tch.at(2);
				if (!float_equality<double>(testing.get_sy_tested_prob(), tch.at(2), 1e-5)||
					!float_equality<double>(testing.get_exp_tested_prob(), tch.at(1), 1e-5) || 
					!float_equality<double>(testing.get_prob_flu_tested(), flu_prob, 1e-5)){
					std::cerr << "Wrong testing values at time " << time << std::endl;
					return false;
				}	
			}
		}
	}
	if (change_count != exp_num_changes){
		std::cerr << "Number of testing values switches not equal expected " << change_count << std::endl;
		return false;
	}	
	return true;
}

/// Test suite for vaccination procedures
bool abm_vaccination()
{
	if (!abm_vaccination_random()){
		std::cerr << "Error in random vaccination functionality" << std::endl;
		return false;
	}
	if (!abm_vaccination_group()){
		std::cerr << "Error in group vaccination functionality" << std::endl;
		return false;
	}
	return true;
}

/// Test for vaccination of random population
bool abm_vaccination_random()
{
	double tol = 1e-3;
	double dt = 0.25, time = 0.0;
	int tmax = 200;
	int initially_infected = 100;	
	int n_vaccinated = 5000;
	int min_vac_age = 12;

	// Regular tests
	ABM abm = create_abm(dt, initially_infected);
	abm.set_random_vaccination(n_vaccinated);
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){
		const std::vector<Agent>& agents = abm.get_vector_of_agents();
		// No testing - no vaccination 
		if (time < infection_parameters.at("start testing")){
			for (const auto& agent : agents){
				if (agent.vaccinated() == true){
					std::cerr << "Agents vaccinated before vaccination is supposed to start" << std::endl;
					return false;
				}
			}
		}

		// Start of vaccination, and the remaining part 
		if (float_equality<double>(time, infection_parameters.at("start testing"), tol)
						|| time > infection_parameters.at("start testing")){
			int actual_vac = 0;
			int death_toll = 0;
			std::vector<int> vac_IDs = {};
			for (const auto& agent : agents){
				if (agent.vaccinated()){
					// Collect 
					++actual_vac;
					vac_IDs.push_back(agent.get_ID());
					// Flag check
					if (!vaccinated_flag_check(agent, min_vac_age)){
						std::cerr << "Agent in an invalid state"  << std::endl;
						return false;
					}
				}
				if (agent.removed_dead()) {
					++death_toll;
				}
			}
			if (float_equality<double>(time, infection_parameters.at("start testing"), tol)) {
				// At the vaccination step the numbers should match exactly
				if (actual_vac != n_vaccinated){			
					std::cerr << "Simulated number of vaccinated agents does not match the model " << actual_vac << " " << n_vaccinated << " " << std::endl;
					//return false;
				}
			} else {
				// Numbers should match, taking into account that now the vaccinated agents can die
				if (actual_vac + death_toll < n_vaccinated){			
					std::cerr << "Simulated number of vaccinated agents does not match the model during the simulation " << (actual_vac + death_toll) << " " << n_vaccinated << " " << std::endl;
					//return false;
				}
			}
			// IDs should be unique
			std::vector<int> orig_vac = vac_IDs;
			auto last = std::unique(vac_IDs.begin(), vac_IDs.end());
   			vac_IDs.erase(last, vac_IDs.end()); 
			if (vac_IDs.size() != orig_vac.size()){			
				std::cerr << "IDs of vaccinated agents are not unique" << std::endl;
				return false;
			}			
			// IDs shouldn't be consecutive - at least not all of them
			std::vector<int> id_diff(orig_vac.size()-1,0); 
			std::transform(std::next(orig_vac.begin()), orig_vac.end(), vac_IDs.begin(), id_diff.begin(), std::minus<int>());
			if (std::all_of(id_diff.begin(), id_diff.end(), [](int x){ return ((x == 1) || (x == -1)); })){
				std::cerr << "IDs of vaccinated agents are not random but sequential" << std::endl;
				return false;
			}
		}
		// Propagate
		time = abm.get_time(); 
		abm.transmit_infection();
	}

	// Exception test - number of vaccinated
	std::runtime_error arg_err("Wrong number to get vaccinated");
	// New instance and initialization
	ABM ex_abm = create_abm(dt, initially_infected);
	const std::vector<Agent>& agents = ex_abm.get_vector_of_agents();
	// This should be enough for "too many to vaccinate"; if it is not - there are no infected and no flu
	// i.e. something is wrong
	n_vaccinated = agents.size();
	ex_abm.set_random_vaccination(n_vaccinated);
	// For information from the exception wrapper
	bool verbose = false;
	for (int ti = 0; ti<=tmax; ++ti){
		time = ex_abm.get_time();
		if (exception_test(verbose, &arg_err, &ABM::transmit_infection, ex_abm)
				&& float_equality<double>(time, infection_parameters.at("start testing"), tol)){
			std::cerr << "Should not throw an exception for too many vaccinated requested - just a warning" << std::endl;
			return false;
		}
		if (ex_abm.get_total_vaccinated() > infection_parameters.at("Maximum number to vaccinate")) {
			std::cerr << "Number of vaccinated agents exceeded maximum allowable number " << std::endl;
			return false;
		}
	}
	return true;
}

/// Test for vaccination of specific population groups
bool abm_vaccination_group()
{
	double tol = 1e-3;
	double dt = 0.25, time = 0.0;
	int tmax = 200;
	int initially_infected = 10000;	
	std::string v_group("hospital employees");
	bool v_verbose = true;
	int min_vac_age = 12;

	//
	// Hospital employees - regular tests
	//
	
	ABM abm = create_abm(dt, initially_infected);
	abm.set_group_vaccination(v_group, v_verbose);
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){

		// No testing - no vaccination 
		if (time < infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			for (const auto& agent : agents){
				if (agent.vaccinated() == true){
					std::cerr << "Agents vaccinated before vaccination is supposed to start" << std::endl;
					return false;
				}
			}
		}

		// Start of vaccination, and the remaining part 
		if (float_equality<double>(time, infection_parameters.at("start testing"), tol)
						|| time > infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			int actual_vac = 0;
			for (const auto& agent : agents){
				if (agent.vaccinated()){
					// Collect 
					++actual_vac;
					// Group check
					if (!agent.hospital_employee()){
						std::cerr << "Agent not in hospital employee group"  << std::endl;
						return false;
					}
					// Flag check - throught the run
					if (!vaccinated_flag_check(agent, min_vac_age)) {
						std::cerr << "Agent in an invalid state"  << std::endl;
						return false;
					}
				}
			}
		}
		// Propagate
		time = abm.get_time(); 
		abm.transmit_infection();
	}

	//
	// School employees - regular tests
	// 

	v_group = "school employees";	
	abm = create_abm(dt, initially_infected);
	abm.set_group_vaccination(v_group, v_verbose);

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){

		// No testing - no vaccination 
		if (time < infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			for (const auto& agent : agents){
				if (agent.vaccinated() == true){
					std::cerr << "Agents vaccinated before vaccination is supposed to start" << std::endl;
					return false;
				}
			}
		}

		// Start of vaccination, and the remaining part 
		if (float_equality<double>(time, infection_parameters.at("start testing"), tol)
						|| time > infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			int actual_vac = 0;
			for (const auto& agent : agents){
				if (agent.vaccinated()){
					// Collect 
					++actual_vac;
					// Group check
					if (!agent.school_employee()){
						std::cerr << "Agent not a school employee"  << std::endl;
						return false;
					}
					// Flag check - throught the run
					if (!vaccinated_flag_check(agent, min_vac_age)){
						std::cerr << "Agent in an invalid state"  << std::endl;
						return false;
					}
				}
			}
		}
		// Propagate
		time = abm.get_time(); 
		abm.transmit_infection();
	}

	//
	// Retirement home employees - regular tests
	// 

	v_group = "retirement home employees";	
	abm = create_abm(dt, initially_infected);
	abm.set_group_vaccination(v_group, v_verbose);

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){

		// No testing - no vaccination 
		if (time < infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			for (const auto& agent : agents){
				if (agent.vaccinated() == true){
					std::cerr << "Agents vaccinated before vaccination is supposed to start" << std::endl;
					return false;
				}
			}
		}

		// Start of vaccination, and the remaining part 
		if (float_equality<double>(time, infection_parameters.at("start testing"), tol)
						|| time > infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			int actual_vac = 0;
			for (const auto& agent : agents){
				if (agent.vaccinated()){
					// Collect 
					++actual_vac;
					// Group check
					if (!agent.retirement_home_employee()){
						std::cerr << "Agent not a retirement home employee"  << std::endl;
						return false;
					}
					// Flag check - throught the run
					if (!vaccinated_flag_check(agent, min_vac_age)){
						std::cerr << "Agent in an invalid state"  << std::endl;
						return false;
					}
				}
			}
		}
		// Propagate
		time = abm.get_time(); 
		abm.transmit_infection();
	}

	//
	// Retirement home resident - regular tests
	// 

	v_group = "retirement home residents";	
	abm = create_abm(dt, initially_infected);
	abm.set_group_vaccination(v_group, v_verbose);

	// Simulation
	for (int ti = 0; ti<=tmax; ++ti){

		// No testing - no vaccination 
		if (time < infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			for (const auto& agent : agents){
				if (agent.vaccinated() == true){
					std::cerr << "Agents vaccinated before vaccination is supposed to start" << std::endl;
					return false;
				}
			}
		}

		// Start of vaccination, and the remaining part 
		if (float_equality<double>(time, infection_parameters.at("start testing"), tol)
						|| time > infection_parameters.at("start testing")){
			const std::vector<Agent>& agents = abm.get_vector_of_agents();
			int actual_vac = 0;
			for (const auto& agent : agents){
				if (agent.vaccinated()){
					// Collect 
					++actual_vac;
					// Group check
					if (!agent.retirement_home_resident()){
						std::cerr << "Agent not a retirement home resident"  << std::endl;
						return false;
					}
					// Flag check - throught the run
					if (!vaccinated_flag_check(agent, min_vac_age)){
						std::cerr << "Agent in an invalid state"  << std::endl;
						return false;
					}
				}
			}
		}
		// Propagate
		time = abm.get_time(); 
		abm.transmit_infection();
	}

	//
	// Exception test - wrong input 
	//
	
	std::invalid_argument arg_err("Wrong name of the group");
	std::string wrong_name("middle school employee");
	// New instance and initialization
	ABM ex_abm = create_abm(dt, initially_infected);
	// For information from the exception wrapper
	bool verbose = false;
	v_verbose = false;
	ex_abm.set_group_vaccination(wrong_name, v_verbose);
	for (int ti = 0; ti<=tmax; ++ti){
		time = ex_abm.get_time();
		if (!exception_test(verbose, &arg_err, &ABM::transmit_infection, ex_abm)
				&& float_equality<double>(time, infection_parameters.at("start testing"), tol)){
			std::cerr << "Failed to throw exception for wrong name of the group to be vaccinated" << std::endl;
			return false;
		}
	}
	return true;
}

// Fetures related to vaccination/reopening studies
bool abm_vac_reopening()
{
	double dt = 0.25;
	int tmax = 50, inf0 = 1;

	// Input files
	std::string fin("test_data/NR_agents.txt");
	std::string hfile("test_data/NR_households.txt");
	std::string sfile("test_data/NR_schools.txt");
	std::string wfile("test_data/NR_workplaces.txt");
	std::string hsp_file("test_data/NR_hospitals.txt");
	std::string rh_file("test_data/NR_retirement_homes.txt");
	std::string cp_file("test_data/NR_carpool.txt");
	std::string pt_file("test_data/NR_public.txt");
	std::string ls_file("test_data/NR_leisure.txt");

	// File with infection parameters
	std::string pfname("test_data/vac_reopen_infection_parameters.txt");
	// Files with age-dependent distributions
	std::string dexp_name("test_data/age_dist_exposed_never_sy.txt");
	std::string dh_name("test_data/age_dist_hospitalization.txt");
	std::string dhicu_name("test_data/age_dist_hosp_ICU.txt");
	std::string dmort_name("test_data/age_dist_mortality.txt");
	// Map for abm loading of distributions
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", dexp_name}, {"hospitalization", dh_name}, 
		  {"ICU", dhicu_name}, {"mortality", dmort_name} };
	// File with time dependent testing parameters
	std::string tfname("test_data/vac_reopen_tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	// The places
	abm.create_households(hfile);
	abm.create_schools(sfile);
	abm.create_workplaces(wfile);
	abm.create_hospitals(hsp_file);
	abm.create_retirement_homes(rh_file);
	abm.create_carpools(cp_file);
	abm.create_public_transit(pt_file);
	abm.create_leisure_locations(ls_file);
	abm.initialize_mobility();
	// The agents
	abm.create_agents(fin, inf0);
	// Initialization for vaccination/reopening studies
	abm.initialize_vac_and_reopening();

	std::vector<Agent>& agents = abm.get_vector_of_agents_non_const();
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();

	// Vaccination info
	int vac0 = infection_parameters.at("initially vaccinated");
	int del_vac = static_cast<int>(infection_parameters.at("vaccination rate")*dt);
	int exp_cur_vac = vac0, cur_vac = 0;	

	// Leisure locations
	double beta_L0 = 0.0399, del_beta = 0.0301; 
	double frac_ini = 0.1, del_frac = 0.6, les_rate = 0.025;  
	double exp_beta = 0.0, exp_frac = 0.0, exp_beta_leisure_out = 0.0;

	for (int ti = 0; ti<=tmax; ++ti) {
		
		// Testing - always equal to 1
		const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();
		Testing testing = abm.get_testing_object();
		double flu_prob = (infection_parameters.at("fraction false positive") + infection_parameters.at("negative tests fraction"))
						*infection_parameters.at("fraction to get tested");
		if (!float_equality<double>(testing.get_sy_tested_prob(), 
								infection_parameters.at("fraction to get tested"), 1e-5)||
			!float_equality<double>(testing.get_exp_tested_prob(), 
								infection_parameters.at("exposed fraction to get tested"), 1e-5) || 
			!float_equality<double>(testing.get_prob_flu_tested(), flu_prob, 1e-5)) {
			std::cerr << "Wrong testing values" << std::endl;
			return false;
		}	
		
		// Vaccination count	
		for (const auto& agent : agents) { 
			if (agent.vaccinated()) {
				++cur_vac;
			}
			// All needs to be tested, or contact traced, or treated due to tracing
			if ((agent.symptomatic() && !agent.tested()) && (!agent.tested_false_negative())
					&& (!agent.tested_covid_positive()) && (!agent.contact_traced())
					&& (!agent.being_treated())) {
				std::cerr << "Symptomatic agent needs to be tested" << std::endl;
				return false;
			}
		}
		if (cur_vac != exp_cur_vac) {
			std::cerr << "Number vaccinated not equal expected" << std::endl;
			return false;
		}

		// Rates and fractions check
		exp_beta = beta_L0 + les_rate*del_beta*abm.get_time();
		exp_beta_leisure_out = exp_beta*infection_parameters.at("fraction estimated infected");
		exp_frac = frac_ini + les_rate*del_frac*abm.get_time();

		// Check rate at each public leisure location
		const std::vector<Leisure>& leisure = abm.get_vector_of_leisure_locations();
		for (const auto& les: leisure) {
			if (les.outside_town()){
				if (!float_equality<double>(les.get_outside_lambda(), exp_beta_leisure_out, 1e-3)){
					std::cerr << "Wrong leisure transmission rate in an outside leisure location "
						<< " "<< exp_beta << " " << les.get_outside_lambda() << " " << exp_beta_leisure_out << std::endl;
					return false;
				}
			}else{
				if (!float_equality<double>(les.get_transmission_rate(), exp_beta, 1e-3)){
					std::cerr << "Wrong public leisure location transmission rate " 
							  << les.get_transmission_rate() << " " << exp_beta << std::endl;
					return false;
				}
			}
		}
		// And the current fraction
		const std::map<std::string, double> cur_infection_parameters = abm.get_infection_parameters();
		if (!float_equality<double>(cur_infection_parameters.at("leisure - fraction"), exp_frac, 1e-2)){
			std::cerr << "Wrong fraction of agents going to leisure locations " << std::endl;
			return false;
		}

		abm.transmit_ideal_testing_vac_reopening();
		
		// Count vaccinated 
		exp_cur_vac += del_vac;
		// Reset counter
		cur_vac = 0;	
	}
	return true;
}

// Vaccination and reopening with initial COVID-19 cases 
bool abm_vac_reopening_seeded()
{
	double dt = 0.25;
	int tmax = 5, inf0 = 1, N_active = 10000;

	// Input files
	std::string fin("test_data/NR_agents.txt");
	std::string hfile("test_data/NR_households.txt");
	std::string sfile("test_data/NR_schools.txt");
	std::string wfile("test_data/NR_workplaces.txt");
	std::string hsp_file("test_data/NR_hospitals.txt");
	std::string rh_file("test_data/NR_retirement_homes.txt");
	std::string cp_file("test_data/NR_carpool.txt");
	std::string pt_file("test_data/NR_public.txt");
	std::string ls_file("test_data/NR_leisure.txt");

	// File with infection parameters
	std::string pfname("test_data/vac_reopen_infection_parameters.txt");
	// Files with age-dependent distributions
	std::string dexp_name("test_data/age_dist_exposed_never_sy.txt");
	std::string dh_name("test_data/age_dist_hospitalization.txt");
	std::string dhicu_name("test_data/age_dist_hosp_ICU.txt");
	std::string dmort_name("test_data/age_dist_mortality.txt");
	// Map for abm loading of distributions
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", dexp_name}, {"hospitalization", dh_name}, 
		  {"ICU", dhicu_name}, {"mortality", dmort_name} };
	// File with time dependent testing parameters	
	std::string tfname("test_data/vac_reopen_tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	// The places
	abm.create_households(hfile);
	abm.create_schools(sfile);
	abm.create_workplaces(wfile);
	abm.create_hospitals(hsp_file);
	abm.create_retirement_homes(rh_file);
	abm.create_carpools(cp_file);
	abm.create_public_transit(pt_file);
	abm.create_leisure_locations(ls_file);
	abm.initialize_mobility();
	// The agents
	abm.create_agents(fin, inf0);
	// Initialization for vaccination/reopening studies
	abm.initialize_vac_and_reopening();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active);

	std::vector<Agent>& agents = abm.get_vector_of_agents_non_const();
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();

	// Counters related to activating COVID-19 cases
	std::map<std::string, int> state_counter = {
			 	{"Total exposed", 0}, {"Total symptomatic", 0},
			 	{"Exposed waiting for test", 0}, 
				{"Symptomatic waiting for test", 0},
				{"Exposed getting tested", 0}, {"Symptomatic getting tested", 0},
			 	{"Exposed waiting for results", 0}, 
				{"Symptomatic waiting for results", 0},
				{"Exposed home isolated", 0}, 
				{"Symptomatic home isolated", 0},
				{"Symptomatic hospitalized", 0},
				{"Symptomatic ICU", 0}};

	// For tracking agents removal from contact tracing
	std::map<int, double> traced_agents;
	int n_traced = 0;	
	for (int ti = 0; ti<=tmax; ++ti) {
		
		abm.transmit_ideal_testing_vac_reopening();
	
		// Check relevant agent properties 
		for (const auto& agent : agents) { 
			const int aID = agent.get_ID();
			
			// Keeping track of contact traced
			if (agent.contact_traced()) {
				// Add to the map with expected time to stop being
				// quarantined if not yet there
				if (traced_agents.find(aID) == traced_agents.end()) {
					traced_agents[aID] = abm.get_time() - dt 
							+ infection_parameters.at("quarantine duration")
							+ infection_parameters.at("quarantine memory");
					++n_traced;
				} else {
					// Check if should have been removed
					if (traced_agents[aID] <= abm.get_time()) {
						std::cerr << "Agent still in the quarantine or subsequent memory period" << std::endl;
						return false;					
					}
				}			
			}
 
			if (agent.infected()) {
				// Count each state, check basic logic
				if (agent.exposed()) {
					++state_counter.at("Total exposed");
					if (agent.tested()) {
						if (agent.tested_awaiting_test() && agent.get_time_of_test() > abm.get_time()) {
							++state_counter.at("Exposed waiting for test");
						} else if (agent.tested_awaiting_results()) {
							++state_counter.at("Exposed waiting for results");
						} else if (agent.tested_awaiting_test() && agent.get_time_of_test() <= abm.get_time()) {
							++state_counter.at("Exposed getting tested");
						} else {
							std::cerr << "Exposed tested in an unknown state" << std::endl;
							return false;
						}
						if (!agent.hospital_employee() && !agent.hospital_non_covid_patient()) {
							if (!agent.home_isolated()) {
								std::cerr << "Exposed and tested regular agent should be home isolated" << std::endl;
								return false;
							} else {
								++state_counter.at("Exposed home isolated");
							}
						}	
					}
				} else if (agent.symptomatic()) {
					++state_counter.at("Total symptomatic");
					if (agent.tested()) {
						if (agent.tested_awaiting_test() && agent.get_time_of_test() > abm.get_time()) {
							++state_counter.at("Symptomatic waiting for test");
						} else if (agent.tested_awaiting_results()) {
							++state_counter.at("Symptomatic waiting for results");
						} else if (agent.tested_awaiting_test() && agent.get_time_of_test() <= abm.get_time()) {
							++state_counter.at("Symptomatic getting tested");
						} else {
							std::cerr << "Symptomatic tested in an unknown state" << std::endl;
							return false;
						}
						if (!agent.hospital_non_covid_patient()) {
							if (!(agent.home_isolated() || agent.being_treated())) {
								std::cerr << "Symptomatic and tested agent should be home isolated or treated" << std::endl;
								return false;
							} else {
								++state_counter.at("Symptomatic home isolated");
							}
						}	
					}
					if (agent.hospitalized()) {
						++state_counter.at("Symptomatic hospitalized");
					} else if (agent.hospitalized_ICU()) {
						++state_counter.at("Symptomatic ICU");
					} else if(agent.home_isolated()) {
						++state_counter.at("Symptomatic home isolated");
					}
				} else {
					std::cerr << "Infected agent neither symptomatic nor exposed" << std::endl;
					return false;
				}
			}
		}
	}
	// Check if all states are present at the second step 
	for (const auto& smap : state_counter) {
		if (smap.second == 0) {
			std::cerr << "Zero agents in state " << smap.first << std::endl;
			return false;
		}		
	}

	std::cout << "Contact traced " << n_traced << " agents." << std::endl;

	return true;
}

// Initial COVID-19 cases, including vaccinated 
bool abm_vac_reopening_seeded_with_vaccinated()
{
	double dt = 0.25;
	int tmax = 5, inf0 = 1, N_active = 30000, N_vac = 10000;
	bool vaccinate = true;
	// Input files
	std::string fin("test_data/NR_agents.txt");
	std::string hfile("test_data/NR_households.txt");
	std::string sfile("test_data/NR_schools.txt");
	std::string wfile("test_data/NR_workplaces.txt");
	std::string hsp_file("test_data/NR_hospitals.txt");
	std::string rh_file("test_data/NR_retirement_homes.txt");
	std::string cp_file("test_data/NR_carpool.txt");
	std::string pt_file("test_data/NR_public.txt");
	std::string ls_file("test_data/NR_leisure.txt");

	// File with infection parameters
	std::string pfname("test_data/vac_reopen_infection_parameters.txt");
	// Files with age-dependent distributions
	std::string dexp_name("test_data/age_dist_exposed_never_sy.txt");
	std::string dh_name("test_data/age_dist_hospitalization.txt");
	std::string dhicu_name("test_data/age_dist_hosp_ICU.txt");
	std::string dmort_name("test_data/age_dist_mortality.txt");
	// Map for abm loading of distributions
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", dexp_name}, {"hospitalization", dh_name}, 
		  {"ICU", dhicu_name}, {"mortality", dmort_name} };
	// File with time dependent testing parameters	
	std::string tfname("test_data/vac_reopen_tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	// The places
	abm.create_households(hfile);
	abm.create_schools(sfile);
	abm.create_workplaces(wfile);
	abm.create_hospitals(hsp_file);
	abm.create_retirement_homes(rh_file);
	abm.create_carpools(cp_file);
	abm.create_public_transit(pt_file);
	abm.create_leisure_locations(ls_file);
	abm.initialize_mobility();
	// The agents
	abm.create_agents(fin, inf0);
	// Initialization for vaccination/reopening studies
	abm.initialize_vac_and_reopening();
	// Create a COVID-19 population
	abm.initialize_active_cases(N_active, vaccinate, N_vac);

	std::vector<Agent>& agents = abm.get_vector_of_agents_non_const();
	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters();

	// Counters related to activating COVID-19 cases
	std::map<std::string, int> state_counter = {
			 	{"Total exposed", 0}, {"Total symptomatic", 0},
			 	{"Exposed waiting for test", 0}, 
				{"Symptomatic waiting for test", 0},
				{"Exposed getting tested", 0}, {"Symptomatic getting tested", 0},
			 	{"Exposed waiting for results", 0}, 
				{"Symptomatic waiting for results", 0},
				{"Exposed home isolated", 0}, 
				{"Symptomatic home isolated", 0},
				{"Symptomatic hospitalized", 0},
				{"Symptomatic ICU", 0}};
	// These will be for agents that are also vaccinated
	std::map<std::string, int> vaccinated_state_counter = {
			 	{"Total exposed", 0}, {"Total symptomatic", 0},
			 	{"Exposed waiting for test", 0}, 
				{"Symptomatic waiting for test", 0},
				{"Exposed getting tested", 0}, {"Symptomatic getting tested", 0},
			 	{"Exposed waiting for results", 0}, 
				{"Symptomatic waiting for results", 0},
				{"Exposed home isolated", 0}, 
				{"Symptomatic home isolated", 0},
				{"Symptomatic hospitalized", 0},
				{"Symptomatic ICU", 0},
				{"Needs next vaccine", 0},
				{"Mobility increased", 0}};

	for (int ti = 0; ti<=tmax; ++ti) {
		
		abm.transmit_ideal_testing_vac_reopening();
	
		// Check relevant agent properties 
		for (const auto& agent : agents) { 
			const int aID = agent.get_ID();
			if (agent.vaccinated()) {
				if (agent.more_active()) {
					++vaccinated_state_counter.at("Mobility increased");
				}
				if (agent.needs_next_vaccination()) {
					++vaccinated_state_counter.at("Needs next vaccine");
				}
			}
			if (agent.infected()) {
				// Count each state, check basic logic
				if (agent.exposed()) {
					++state_counter.at("Total exposed");
					if (agent.vaccinated()) {
						++vaccinated_state_counter.at("Total exposed");
					}
					if (agent.tested()) {
						if (agent.tested_awaiting_test() && agent.get_time_of_test() > abm.get_time()) {
							++state_counter.at("Exposed waiting for test");
							if (agent.vaccinated()) {
								++vaccinated_state_counter.at("Exposed waiting for test");
							}
						} else if (agent.tested_awaiting_results()) {
							++state_counter.at("Exposed waiting for results");
							if (agent.vaccinated()) {
								++vaccinated_state_counter.at("Exposed waiting for results");
							}
						} else if (agent.tested_awaiting_test() && agent.get_time_of_test() <= abm.get_time()) {
							++state_counter.at("Exposed getting tested");
							if (agent.vaccinated()) {
								++vaccinated_state_counter.at("Exposed getting tested");
							}
						} else {
							std::cerr << "Exposed tested in an unknown state" << std::endl;
							return false;
						}
						if (!agent.hospital_employee() && !agent.hospital_non_covid_patient()) {
							if (!agent.home_isolated()) {
								std::cerr << "Exposed and tested regular agent should be home isolated" << std::endl;
								return false;
							} else {
								++state_counter.at("Exposed home isolated");
								if (agent.vaccinated()) {
									++vaccinated_state_counter.at("Exposed home isolated");
								}
							}
						}	
					}
				} else if (agent.symptomatic()) {
					++state_counter.at("Total symptomatic");
					if (agent.vaccinated()) {
						++vaccinated_state_counter.at("Total symptomatic");
					}
					if (agent.tested()) {
						if (agent.tested_awaiting_test() && agent.get_time_of_test() > abm.get_time()) {
							++state_counter.at("Symptomatic waiting for test");
							if (agent.vaccinated()) {
								++vaccinated_state_counter.at("Symptomatic waiting for test");
							}
						} else if (agent.tested_awaiting_results()) {
							++state_counter.at("Symptomatic waiting for results");
							if (agent.vaccinated()) {
								++vaccinated_state_counter.at("Symptomatic waiting for results");
							}
						} else if (agent.tested_awaiting_test() && agent.get_time_of_test() <= abm.get_time()) {
							++state_counter.at("Symptomatic getting tested");
							if (agent.vaccinated()) {
								++vaccinated_state_counter.at("Symptomatic getting tested");
							}
						} else {
							std::cerr << "Symptomatic tested in an unknown state" << std::endl;
							return false;
						}
						if (!agent.hospital_non_covid_patient()) {
							if (!(agent.home_isolated() || agent.hospitalized() || agent.hospitalized_ICU())) {
								std::cerr << "Symptomatic and tested agent should be home isolated" << std::endl;
								return false;
							} else {
								++state_counter.at("Symptomatic home isolated");
								if (agent.vaccinated()) {
									++vaccinated_state_counter.at("Symptomatic home isolated");
								}
							}
						}	
					}
					if (agent.hospitalized()) {
						++state_counter.at("Symptomatic hospitalized");
						if (agent.vaccinated()) {
							++vaccinated_state_counter.at("Symptomatic hospitalized");
						}
					} else if (agent.hospitalized_ICU()) {
						++state_counter.at("Symptomatic ICU");
						if (agent.vaccinated()) {
							++vaccinated_state_counter.at("Symptomatic ICU");
						}
					} 
				} else {
					std::cerr << "Infected agent neither symptomatic nor exposed" << std::endl;
					return false;
				}
			}
		}
	}
	// Check if all states are present at the second step 
	for (const auto& smap : state_counter) {
		if (smap.second == 0) {
			std::cerr << "Zero agents in state " << smap.first << std::endl;
			return false;
		}		
	}
	return true;
}

// Common operations for creating the ABM interface
ABM create_abm(const double dt, int inf0)
{
	// Input files
	std::string fin("test_data/NR_agents.txt");
	std::string hfile("test_data/NR_households.txt");
	std::string sfile("test_data/NR_schools.txt");
	std::string wfile("test_data/NR_workplaces.txt");
	std::string hsp_file("test_data/NR_hospitals.txt");
	std::string rh_file("test_data/NR_retirement_homes.txt");
	std::string cp_file("test_data/NR_carpool.txt");
	std::string pt_file("test_data/NR_public.txt");
	std::string ls_file("test_data/NR_leisure.txt");

	// File with infection parameters
	std::string pfname("test_data/infection_parameters.txt");
	// Files with age-dependent distributions
	std::string dexp_name("test_data/age_dist_exposed_never_sy.txt");
	std::string dh_name("test_data/age_dist_hospitalization.txt");
	std::string dhicu_name("test_data/age_dist_hosp_ICU.txt");
	std::string dmort_name("test_data/age_dist_mortality.txt");
	// Map for abm loading of distributions
	std::map<std::string, std::string> dfiles = 
		{ {"exposed never symptomatic", dexp_name}, {"hospitalization", dh_name}, 
		  {"ICU", dhicu_name}, {"mortality", dmort_name} };
	// File with time dependent testing parameters
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	// First the places
	abm.create_households(hfile);
	abm.create_schools(sfile);
	abm.create_workplaces(wfile);
	abm.create_hospitals(hsp_file);
	abm.create_retirement_homes(rh_file);
	abm.create_carpools(cp_file);
	abm.create_public_transit(pt_file);
	abm.create_leisure_locations(ls_file);

	abm.initialize_mobility();
	abm.create_agents(fin, inf0);

	return abm;	
}

// Check if a newly vaccinated agent does not 
// fall into any of these categories
bool vaccinated_flag_check(const Agent& agent, const int min_vac_age)
{
	if (agent.get_age() < min_vac_age) {
		return false;	
	}
	return true;
}
