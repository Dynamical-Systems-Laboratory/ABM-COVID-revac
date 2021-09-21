#include "abm_tests.h"

/***************************************************** 
 *
 * Test suite for creation of ABM objects
 *
 ******************************************************/

// Tests
bool create_households_test();
bool create_schools_test();
bool wrong_school_type_test();
bool create_workplaces_test();
bool create_hospitals_test();
bool create_retirement_homes_test();
bool create_leisure_locations_test();
bool create_carpools_test();
bool create_public_transit_test();
bool create_agents_test();
bool create_agents_file_test();
bool vac_reopen_setup_test();
bool create_active_for_vac_reopen_test();
bool create_active_w_vaccinated();

// Supporting functions
bool compare_places_files(std::string fname_in, std::string fname_out, 
				const std::vector<double> infection_parameters, const bool has_type = false, 
				const bool has_extra = false, const bool has_print_type = false);
bool compare_workplaces_files(std::string fname_in, std::string fname_out, 
				const std::map<std::string, double>& infection_parameters);
bool compare_transit_files(std::string fname_in, std::string fname_out, 
				const std::vector<double> infection_parameters);
bool compare_agents_files(std::string fname_in, std::string fname_out);
bool correctly_registered(const ABM, const std::vector<std::vector<int>>, 
							const std::vector<std::vector<int>>, std::string, 
							const std::string, const std::string, const int);
bool check_initially_infected(const Agent& agent, const Flu& flu, int& n_exposed_never_sy,
								const std::map<std::string, double> infection_parameters);
bool check_fractions(int, int, double, std::string, bool need_ge = false);

int main()
{
	test_pass(create_households_test(), "Household creation");
	test_pass(create_schools_test(), "School creation");
	test_pass(wrong_school_type_test(), "Wrong school type detection");
	test_pass(create_workplaces_test(), "Workplace creation");
	test_pass(create_hospitals_test(), "Hospitals creation");
	test_pass(create_retirement_homes_test(), "Retirement homes creation");
	test_pass(create_leisure_locations_test(), "Leisure locations creation");
	test_pass(create_carpools_test(), "Carpools creation");
	test_pass(create_public_transit_test(), "Public transit creation");
	test_pass(create_agents_test(), "Agent creation");
	test_pass(create_agents_file_test(), "Agent creation - file");
	test_pass(vac_reopen_setup_test(), "Initialization for vaccination/reopening studies");
	test_pass(create_active_for_vac_reopen_test(), "Initialization of active COVID-19 cases for vaccination/reopening studies");
	test_pass(create_active_w_vaccinated(), "Initialization of active COVID-19 cases with vaccinated agents");
}

// Checks household creation from file
bool create_households_test()
{
	// Create households and save the information
	std::string fin("test_data/NR_households.txt");
	std::string fout("test_data/houses_out.txt");

	// Model parameters
	// Time step
	double dt = 2.0;
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
	// File with time-dependent testing parameters 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	abm.create_households(fin);
	abm.print_households(fout);
	
	// Vector of infection parameters 
	// (order as in output file: ck, beta,  alpha, beta_ih)
	std::vector<double> infection_parameters = {2.0, 0.69, 0.80, 0.48}; 

	// Check if correct, hardcoded for places properties
	if (!compare_places_files(fin, fout, infection_parameters)){
		std::cerr << "Error in household creation" << std::endl;
		return false;
	}
	return true;
}

// Checks school creation from file
bool create_schools_test()
{
	// Create schools and save the information
	std::string fin("test_data/NR_schools.txt");
	std::string fout("test_data/schools_out.txt");

	double dt = 1.0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	abm.create_schools(fin);
	abm.print_schools(fout);

	bool has_type = true;
	// Vector of infection parameters 
	// (order as in output file: ck, beta, beta for employee, psi for employee, 
	// psi for middle)
	std::vector<double> infection_parameters = {2.0, 1.33, 0.66, 0.2, 0.1}; 

	// Check if correct, hardcoded for places properties
	if (!compare_places_files(fin, fout, infection_parameters, has_type)){
		std::cerr << "Error in school creation" << std::endl;
		return false;
	}
	return true;
}

// Tests if exception is triggered upon entering a wrong school type
bool wrong_school_type_test()
{
	bool verbose = true;
	const std::invalid_argument invarg("Wrong type");

	std::string fin("test_data/schools_wrong_type.txt");
	double dt = 2.0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);
	
	if (!exception_test(verbose, &invarg, &ABM::create_schools, abm, fin)){
		std::cerr << "Wrong school type not recognized as an error" << std::endl;
		return false;
	}
	return true;
}

// Checks workplace creation from file
bool create_workplaces_test()
{
	// Create workplaces and save the information
	std::string fin("test_data/NR_workplaces.txt");
	std::string fout("test_data/workplaces_out.txt");

	// Model parameters
	// Time step
	double dt = 2.0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);
	
	abm.create_workplaces(fin);
	abm.print_workplaces(fout);
	
 	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters(); 
	// Check if correct, hardcoded for places properties
	if (!compare_workplaces_files(fin, fout, infection_parameters)){
		std::cerr << "Error in workplace creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks hospital creation from file
bool create_hospitals_test()
{
	// Create workplaces and save the information
	std::string fin("test_data/NR_hospitals.txt");
	std::string fout("test_data/hospitals_out.txt");

	// Model parameters
	// Time step
	double dt = 2.0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	abm.create_hospitals(fin);
	abm.print_hospitals(fout);
	
	// Vector of infection parameters 
	// (order as in output file: ck, betas for each category)
	std::vector<double> infection_parameters = {2.0, 1.28, 1.38, 1.75, 1.02, 1.34}; 

	// Check if correct, hardcoded for places properties
	if (!compare_places_files(fin, fout, infection_parameters)){
		std::cerr << "Error in hospital creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks retirement home creation
bool create_retirement_homes_test()
{
	// Create workplaces and save the information
	std::string fin("test_data/NR_retirement_homes.txt");
	std::string fout("test_data/ret_homes_out.txt");

	// Model parameters
	// Time step
	double dt = 2.0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	abm.create_retirement_homes(fin);
	abm.print_retirement_home(fout);

	// Vector of infection parameters 
	// (order as in output file: ck, beta resident, beta employee, beta
	// home isolated, psi employee)
	std::vector<double> infection_parameters = {2.0, 0.69, 0.66, 0.48, 0.0}; 

	// Check if correct, hardcoded for places properties
	if (!compare_places_files(fin, fout, infection_parameters)){
		std::cerr << "Error in retirement home creation" << std::endl;
		return false;
	}
	return true;
}

// Checks creation of leisure locations from file
bool create_leisure_locations_test()
{
	// Create leisure locations and save the information
	std::string fin("test_data/NR_leisure.txt");
	std::string fout("test_data/leisure_out.txt");

	// Model parameters
	// Time step
	double dt = 2.0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	abm.create_leisure_locations(fin);
	abm.print_leisure_locations(fout);
	
	// Vector of infection parameters 
	// (order as in output file: ck, beta)
	std::vector<double> infection_parameters = {2.0, 0.07}; 

	// Check if correct, hardcoded for places properties
	if (!compare_places_files(fin, fout, infection_parameters, true, false, true)){
		std::cerr << "Error in leisure location creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks creation of carpools from file
bool create_carpools_test()
{
	// Create carpools and save the information
	std::string fin("test_data/NR_carpool.txt");
	std::string fout("test_data/carpool_out.txt");

	// Model parameters
	// Time step
	double dt = 2.0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	abm.create_carpools(fin);
	abm.print_transit(fout, "carpool");
	
	// Vector of infection parameters 
	// (order as in output file: ck, beta)
	std::vector<double> infection_parameters = {2.0, 0.9, 0.1}; 

	// Check if correct, hardcoded for places properties
	if (!compare_transit_files(fin, fout, infection_parameters)){
		std::cerr << "Error in carpools creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks creation of public transit from file
bool create_public_transit_test()
{
	// Create public transit objects and save the information
	std::string fin("test_data/NR_public.txt");
	std::string fout("test_data/public_out.txt");

	// Model parameters
	// Time step
	double dt = 2.0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");
	// File with vaccination parameters 	
	std::string vfname("test_data/vaccination_parameters.txt");
	// Directory with vaccination data files 	
	std::string vdata("test_data/");

	ABM abm(dt, pfname, dfiles, tfname, vfname, vdata);

	abm.create_public_transit(fin);
	abm.print_transit(fout, "public");
	
	// Vector of infection parameters 
	// (order as in output file: ck, beta)
	std::vector<double> infection_parameters = {2.0, 0.0265, 0.1}; 

	// Check if correct, hardcoded for places properties
	if (!compare_transit_files(fin, fout, infection_parameters)){
		std::cerr << "Error in public transit creation" << std::endl;
		return false;
	}
	return true;	
}

// Checks agents creation from file including proper 
// distribution into places 
bool create_agents_test()
{
	double dt = 0.25;
	int inf0 = 763, inf0_count = 0;
	int n_exposed_never_sy = 0;
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
	// File with 	
	std::string tfname("test_data/tests_with_time.txt");

	// Acceptable transit modes to work
	std::vector<std::string> all_travel_modes = {"car", "carpool", "public", "walk",
													"other", "wfh", "None"};
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
	// Then the agents
	abm.create_agents(fin, inf0);
	
	const std::vector<Agent>& agents = abm.get_vector_of_agents_non_const();
	const std::vector<Household>& households = abm.get_vector_of_households();
	const std::vector<School>& schools = abm.get_vector_of_schools();
	const std::vector<Workplace>& workplaces = abm.get_vector_of_workplaces();
	const std::vector<Hospital>& hospitals = abm.get_vector_of_hospitals();
	const std::vector<RetirementHome>& retirement_homes = abm.get_vector_of_retirement_homes();
	const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
	const std::vector<Transit>& public_transit = abm.get_vector_of_public_transit();
	const std::vector<Leisure>& leisure = abm.get_vector_of_leisure_locations();

	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters(); 
	Flu& flu = abm.get_flu_object();

	// Check registration and count initially infected as created by ABM
	for (const auto& agent : agents){ 
		const int aID = agent.get_ID();
		const std::string agent_transit = agent.get_work_travel_mode();
		if (std::find(all_travel_modes.begin(), all_travel_modes.end(), agent_transit)
						== all_travel_modes.end()) {
			std::cerr << "Invalid travel mode" << std::endl;
			return false;
		}
		if ((!agent.works()) && (!agent.hospital_employee())){
			if ((agent_transit != "None") || 
				!(float_equality<double>(agent.get_work_travel_time(), 0.0,  1e-5))) {
				std::cerr << "Agent that is not employed has non-zero travel time or invalid travel mode" << std::endl;
				return false;
			}
		}
		if (agent.infected()){
			++inf0_count;
			if (!check_initially_infected(agent, flu, n_exposed_never_sy, infection_parameters)){
				std::cerr << "Error in initialization of infected agent" << std::endl;
				return false;	
			}	
		} 
		if (agent.student()){
			if (!find_in_place<School>(schools, aID, agent.get_school_ID())){
				std::cerr << "Agent not registered in a school" << std::endl;
				return false;
			}
		}

		// Works from home settings
		if (agent.works_from_home()) {
			if ((!agent.works()) 
							|| !(float_equality<double>(agent.get_work_travel_time(), 0.0,  1e-5))
							|| (agent.get_work_travel_mode() != "wfh")) {
				std::cerr << "Agent that works from home has invalid properties" << std::endl;
				return false;
			}
		}

		if (agent.works() && !agent.works_from_home()){
			if ((agent.get_work_travel_time() <= 0.0) || (agent_transit  == "None")
						|| (agent_transit  == "wfh")) {
				std::cerr << "Agent that works has invalid travel-related properties" << std::endl;
				return false;
			}
			if (agent.school_employee()){
				if (!find_in_place<School>(schools, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a school as an employee" << std::endl;
					return false;
				}
			}else if (agent.retirement_home_employee()){
				if (!find_in_place<RetirementHome>(retirement_homes, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a retirement home as an employee" << std::endl;
					return false;
				}		
			} else {
				if (!find_in_place<Workplace>(workplaces, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a workplace" << std::endl;
					return false;
				}
			}
		}

		// Household or retirement home
		if (agent.retirement_home_resident()){
			if (!find_in_place<RetirementHome>(retirement_homes, aID, agent.get_household_ID())){
				std::cerr << "Agent not registered in a retirement home" << std::endl;
				return false;
			}	
		} else if (agent.hospital_non_covid_patient()){
			if (!find_in_place<Hospital>(hospitals, aID, agent.get_hospital_ID())){
				std::cerr << "Hospital patient without covid is not registered in a hospital" << std::endl;
				return false;
			}
		} else {
			if (!find_in_place<Household>(households, aID, agent.get_household_ID())){
				std::cerr << "Agent not registered in a household" << std::endl;
				return false;
			}
		}
		// Hospital employee
		if (agent.hospital_employee()){
			if (!find_in_place<Hospital>(hospitals, aID, agent.get_hospital_ID())){
				std::cerr << "Hospital employee is not registered in a hospital" << std::endl;
				return false;
			}
			if ((agent.get_work_travel_time() <= 0.0) || (agent_transit  == "None")
						|| (agent_transit  == "wfh")) {
				std::cerr << "Agent that works in a hospital has invalid travel-related properties" << std::endl;
				return false;
			}
		}

		// Transit
		if (agent.get_work_travel_mode() == "carpool") {
			if (!find_in_place<Transit>(carpools, aID, agent.get_carpool_ID())){
				std::cerr << "Agent not registered in a carpool" << std::endl;
				return false;
			}
		}
		if (agent.get_work_travel_mode() == "public") {
			if (!find_in_place<Transit>(public_transit, aID, agent.get_public_transit_ID())){
				std::cerr << "Agent not registered in a public transit" << std::endl;
				return false;
			}
		}
	}

	if (inf0 != inf0_count){
		std::cerr << "Initially infected agent numbers don't match expectations" << std::endl;
		return false;
	}
	// Fraction that never develops symptoms
	if (!check_fractions(n_exposed_never_sy, inf0_count, 0.1*infection_parameters.at("fraction exposed never symptomatic"), 
							"Wrong percentage of exposed agents never developing symptoms", true)){
		return false;
	}
	return true;
}

// Checks agents creation from file including proper 
// distribution into places - version with agents initialized as infected from file 
bool create_agents_file_test()
{
	double dt = 0.25;
	int inf0 = 80, inf0_count = 0;
	int n_exposed_never_sy = 0;
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
	// File with 	
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

	// Then the agents
	abm.create_agents(fin);

	const std::vector<Agent>& agents = abm.get_vector_of_agents_non_const();
	const std::vector<Household>& households = abm.vector_of_households();
	const std::vector<School>& schools = abm.vector_of_schools();
	const std::vector<Workplace>& workplaces = abm.vector_of_workplaces();
	const std::vector<Hospital>& hospitals = abm.vector_of_hospitals();
	const std::vector<RetirementHome>& retirement_homes = abm.vector_of_retirement_homes();
	const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
	const std::vector<Transit>& public_transit = abm.get_vector_of_public_transit();
	const std::vector<Leisure>& leisure = abm.get_vector_of_leisure_locations();

	const std::map<std::string, double> infection_parameters = abm.get_infection_parameters(); 
	Flu& flu = abm.get_flu_object();

	// Acceptable transit modes to work
	std::vector<std::string> all_travel_modes = {"car", "carpool", "public", "walk",
													"other", "wfh", "None"};

	// Load the agent file too and check correspondence of the entries
	std::ifstream input(fin);
	// Check registration and count initially infected as created by ABM
	for (const auto& agent : agents){ 
		const int aID = agent.get_ID();
		const std::string agent_transit = agent.get_work_travel_mode();
		if (std::find(all_travel_modes.begin(), all_travel_modes.end(), agent_transit)
						== all_travel_modes.end()) {
			std::cerr << "Invalid travel mode" << std::endl;
			return false;
		}
		if ((!agent.works()) && (!agent.hospital_employee())){
			if ((agent_transit != "None") || 
				!(float_equality<double>(agent.get_work_travel_time(), 0.0,  1e-5))) {
				std::cerr << "Agent that is not employed has non-zero travel time or invalid travel mode" << std::endl;
				return false;
			}
		}

		// Read and parse a line from file that should correspond to this agent
		std::vector<std::string> line(22);
		for (auto& entry : line){
			input >> entry;
		}
				
		if (agent.infected()){
			++inf0_count;
			if (line.at(14) != "1"){
				std::cerr << "This agent was not supposed to be infected" << std::endl;
				return false;
			}
			if (!check_initially_infected(agent, flu, n_exposed_never_sy, infection_parameters)){
				std::cerr << "Error in initialization of infected agent" << std::endl;
				return false;	
			}	
		}

		if (agent.student()){
			if (line.at(0) != "1"){
				std::cerr << "This agent was not supposed to be a student" << std::endl;
				return false;
			}
				if (agent.get_school_ID() != std::stoi(line.at(7))){
					std::cerr << "Loaded school and ID from file don't match" << std::endl;
					return false;
				}
			if (!find_in_place<School>(schools, aID, agent.get_school_ID())){
				std::cerr << "Agent not registered in a school" << std::endl;
				return false;
			}
		}

		// Works from home settings
		if (agent.works_from_home()) {
			if ((!agent.works()) 
							|| !(float_equality<double>(agent.get_work_travel_time(), 0.0,  1e-5))
							|| (agent.get_work_travel_mode() != "wfh")) {
				std::cerr << "Agent that works from home has invalid properties" << std::endl;
				return false;
			}
		}

		if (agent.works() && !agent.works_from_home()){
			if ((agent.get_work_travel_time() <= 0.0) || (agent_transit  == "None")
						|| (agent_transit  == "wfh")) {
				std::cerr << "Agent that works has invalid travel-related properties" << std::endl;
				return false;
			}
			if (agent.school_employee()){
				if (line.at(10) != "1"){
					std::cerr << "This agent was not supposed to work at a school" << std::endl;
					return false;
				}
				if (agent.get_work_ID() != std::stoi(line.at(18))){
					std::cerr << "Loaded school as workplace ID and ID from file don't match" << std::endl;
					return false;
				}
				if (!find_in_place<School>(schools, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a school as an employee" << std::endl;
					return false;
				}
			}else if (agent.retirement_home_employee()){
				if (line.at(9) != "1"){
					std::cerr << "This agent was not supposed to work at a retirement home" << std::endl;
					return false;
				}
				if (agent.get_work_ID() != std::stoi(line.at(18))){
					std::cerr << "Loaded RH as a workplace ID and ID from file don't match" << std::endl;
					return false;
				}
				if (!find_in_place<RetirementHome>(retirement_homes, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a retirement home as an employee" << std::endl;
					return false;
				}		
			} else {
				if (line.at(1) != "1"){
					std::cerr << "This agent was not supposed to work at a general workplace" << std::endl;
					return false;
				}
				if (agent.get_work_ID() != std::stoi(line.at(11))){
					std::cerr << "Loaded workplace ID and ID from file don't match" << std::endl;
					return false;
				}
				if (!find_in_place<Workplace>(workplaces, aID, agent.get_work_ID())){
					std::cerr << "Agent not registered in a workplace" << std::endl;
					return false;
				}
			}
		}

		// Household or retirement home
		if (agent.retirement_home_resident()){
			if (line.at(8) != "1"){
				std::cerr << "This agent was not supposed to live in a retirement home" << std::endl;
				return false;
			}
			if (agent.get_household_ID() != std::stoi(line.at(5))){
				std::cerr << "Loaded RH ID (as residence) and ID from file don't match" << std::endl;
				return false;
			}
			if (!find_in_place<RetirementHome>(retirement_homes, aID, agent.get_household_ID())){
				std::cerr << "Agent not registered in a retirement home" << std::endl;
				return false;
			}	
		} else if (agent.hospital_non_covid_patient()){
			if (line.at(6) != "1"){
				std::cerr << "This agent was not supposed to be a non-COVID hospital patient" << std::endl;
				return false;
			}
			if (agent.get_hospital_ID() != std::stoi(line.at(13))){
				std::cerr << "Loaded hospital ID and ID from file don't match" << std::endl;
				return false;
			}
			if (!find_in_place<Hospital>(hospitals, agent.get_ID(), agent.get_hospital_ID())){
				std::cerr << "Hospital patient without covid is not registered in a hospital" << std::endl;
				return false;
			}
		} else {
			if (agent.get_household_ID() != std::stoi(line.at(5))){
				std::cerr << "Loaded household ID and ID from file don't match" << std::endl;
				return false;
			}
			if (!find_in_place<Household>(households, aID, agent.get_household_ID())){
				std::cerr << "Agent not registered in a household" << std::endl;
				return false;
			}
		}

		// Hospital employee
		if (agent.hospital_employee()){
			if (line.at(12) != "1"){
				std::cerr << "This agent was not supposed to work at a hospital" << std::endl;
				return false;
			}
			if (agent.get_hospital_ID() != std::stoi(line.at(13))){
				std::cerr << "Hospital employee: loaded hospital ID and ID from file don't match" << std::endl;
				return false;
			}
			if (!find_in_place<Hospital>(hospitals, agent.get_ID(), agent.get_hospital_ID())){
				std::cerr << "Hospital employee is not registered in a hospital" << std::endl;
				return false;
			}
			if ((agent.get_work_travel_time() <= 0.0) || (agent_transit  == "None")
						|| (agent_transit  == "wfh")) {
				std::cerr << "Agent that works in a hospital has invalid travel-related properties" << std::endl;
				return false;
			}
		}

		// Transit
		if (agent.get_work_travel_mode() == "carpool") {
			if (!find_in_place<Transit>(carpools, aID, agent.get_carpool_ID())){
				std::cerr << "Agent not registered in a carpool" << std::endl;
				return false;
			}
		}
		if (agent.get_work_travel_mode() == "public") {
			if (!find_in_place<Transit>(public_transit, aID, agent.get_public_transit_ID())){
				std::cerr << "Agent not registered in a public transit" << std::endl;
				return false;
			}
		}
	}

	if (inf0 != inf0_count){
		std::cerr << "Initially infected agent numbers don't match expectations " << std::endl;
		return false;
	}
	// Fraction that never develops symptoms
	if (!check_fractions(n_exposed_never_sy, inf0_count, 0.1*infection_parameters.at("fraction exposed never symptomatic"), 
							"Wrong percentage of exposed agents never developing symptoms", true)){
		return false;
	}
	return true;
}

// Tests setting up special parameters for vaccination/reopening studies 
bool vac_reopen_setup_test()
{
	double dt = 0.25;
	int inf0 = 10;

	// Expected parameters
	int exp_num_vac = 1230;
	double exp_frac_flu = 0.113, exp_test_flu = 0.45;

	double exp_beta_sch_students = 0.4655, exp_beta_sch_emp = 0.231;
	double exp_beta_work_out = 0.0285; 
	double exp_beta_carpool = 0.513, exp_beta_transit = 0.015535;
	double exp_beta_leisure = 0.0399, exp_beta_leisure_out = 0.001995;	

	int num_vac = 0, num_flu = 0, inf0_count = 0;
	int num_flu_tested = 0.0; 

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
	// File with 	
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
	const std::vector<School>& schools = abm.vector_of_schools();
	const std::vector<Workplace>& workplaces = abm.vector_of_workplaces();
	const std::vector<Transit>& carpools = abm.get_vector_of_carpools();
	const std::vector<Transit>& public_transit = abm.get_vector_of_public_transit();
	const std::vector<Leisure>& leisure = abm.get_vector_of_leisure_locations();

	// Check relevant agent properties 
	for (const auto& agent : agents) { 
		const int aID = agent.get_ID();
		if (agent.vaccinated()) {
			++num_vac;
		}
		if (agent.symptomatic_non_covid()) {
			++num_flu;
			if (agent.tested()) {
				++num_flu_tested;
			} 
		} else if (agent.tested()) {
			std::cerr << "Only symptomatic-non COVID agents can have testing set at initialization" << std::endl;
			return false;
		}
		if (agent.infected()) {
			++inf0_count;
		}
	}
	if (static_cast<int>(infection_parameters.at("initially vaccinated")) != num_vac) {
		std::cerr << "Initial number of vaccinated agents does not match expectations" << std::endl;
		return false;
	}
	if (num_flu == 0) {
		std::cerr << "No agents with flu" << std::endl;
		return false;
	}
	if (!check_fractions(num_flu_tested, num_flu, exp_test_flu, 
							"Wrong percentage of agents with flu that will be tested")){
		return false;
	}
	if (inf0 != inf0_count) {
		std::cerr << "Initially infected agent numbers don't match expectations" << std::endl;
		return false;
	}

	// Parameter check
	for (const auto& school : schools) {
		if (!float_equality<double>(school.get_transmission_rate(), exp_beta_sch_students, 1e-5)){
			std::cerr << "Wrong school transmission rate for students" << std::endl;
			return false;
		}
		if (!float_equality<double>(school.get_employee_transmission_rate(), exp_beta_sch_emp, 1e-5)){
			std::cerr << "Wrong school transmission rate for employees" << std::endl;
			return false;
		}
	}
	for (const auto& work : workplaces) {
		if (work.outside_town()) {
			if (!float_equality<double>(work.get_outside_lambda(), exp_beta_work_out, 1e-5)){
				std::cerr << "Wrong workplace transmission rate in an outside workplace" << std::endl;
				return false;
			}
		} else {
			double occ_beta = 0;
			if (work.get_type() == "A") {
				occ_beta = infection_parameters.at("management science art transmission rate");
			} else if (work.get_type() == "B") {
				occ_beta = infection_parameters.at("service occupation transmission rate");
			} else if  (work.get_type() == "C") {
				occ_beta = infection_parameters.at("sales office transmission rate");
			} else if (work.get_type() == "D") {
				occ_beta = infection_parameters.at("construction maintenance transmission rate");
			} else if (work.get_type() == "E") {
				occ_beta = infection_parameters.at("production transportation transmission rate");
			} else if (work.get_type() == "outside") {
				occ_beta = 1.0;
			} else {
				std::cerr << "Unsupported workplace type " << work.get_type() << std::endl;
				return false;
			}
			occ_beta *= infection_parameters.at("fraction of phase 4 businesses");
			if (!float_equality<double>(work.get_transmission_rate(), occ_beta, 1e-5)){
				std::cerr << "Wrong workplace transmission rate in an in-town workplace " << work.get_transmission_rate() << " " << occ_beta << std::endl;
				return false;
			}
		}
	}
	for (const auto& cpool : carpools) {
		if (!float_equality<double>(cpool.get_transmission_rate(), exp_beta_carpool, 1e-5)){
			std::cerr << "Wrong carpool transmission rate" << std::endl;
			return false;
		}
	}
	for (const auto& transit : public_transit) {
		if (!float_equality<double>(transit.get_transmission_rate(), exp_beta_transit, 1e-5)){
			std::cerr << "Wrong transit transmission rate" << std::endl;
			return false;
		}
	}
	for (const auto& les: leisure) {
		if (les.outside_town()){
			if (!float_equality<double>(les.get_outside_lambda(), exp_beta_leisure_out, 1e-5)){
				std::cerr << "Wrong leisure transmission rate in an outside leisure location "<< std::endl;
				return false;
			}
		}else{
			if (!float_equality<double>(les.get_transmission_rate(), exp_beta_leisure, 1e-5)){
				std::cerr << "Wrong public leisure location transmission rate" << std::endl;
				return false;
			}
		}
	}
	return true;
}

// Tests setting up special parameters for vaccination/reopening studies 
bool create_active_for_vac_reopen_test()
{
	double dt = 0.25;
	int inf0 = 10, N_active = 1000;

	// Expected parameters
	int exp_num_vac = 1230;
	double exp_frac_flu = 0.113, exp_test_flu = 0.45;

	double exp_beta_sch_students = 0.4655, exp_beta_sch_emp = 0.231;
	double exp_beta_work = 0.3762, exp_beta_work_out = 0.00057; 
	double exp_beta_carpool = 0.513, exp_beta_transit = 0.015535;
	double exp_beta_leisure = 0.0399;	

	int num_vac = 0, num_flu = 0, inf0_count = 0;
	int num_flu_tested = 0.0; 

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
	// File with 	
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
				{"Symptomatic home isolated", 0}};

	// Check relevant agent properties 
	for (const auto& agent : agents) { 
		const int aID = agent.get_ID();
		if (agent.vaccinated()) {
			++num_vac;
		}
		if (agent.symptomatic_non_covid()) {
			++num_flu;
			if (agent.tested()) {
				++num_flu_tested;
			} 
		} 		
		if (agent.infected()) {
			++inf0_count;
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
						if (!(agent.home_isolated() || agent.hospitalized() || agent.hospitalized_ICU())) {
							std::cerr << "Symptomatic and tested agent should be home isolated" << std::endl;
							return false;
						} else {
							++state_counter.at("Symptomatic home isolated");
						}
					}	
				}
			} else {
				std::cerr << "Infected agent neither symptomatic nor exposed" << std::endl;
				return false;
			}
		}
	}

	// All states need to be larger than 0
	for (const auto& smap : state_counter) {
		if (smap.second == 0) {
			std::cerr << "Zero agents in state " << smap.first << std::endl;
			return false;
		}		
	}
	if (static_cast<int>(infection_parameters.at("initially vaccinated")) != num_vac) {
		std::cerr << "Initial number of vaccinated agents does not match expectations" << std::endl;
		return false;
	}
	if (num_flu == 0) {
		std::cerr << "No agents with flu" << std::endl;
		return false;
	}
	if (!check_fractions(num_flu_tested, num_flu, exp_test_flu, 
							"Wrong percentage of agents with flu that will be tested")){
		return false;
	}
	if (inf0 + N_active != inf0_count) {
		std::cerr << "Initially infected agent numbers don't match expectations" << std::endl;
		return false;
	}

	return true;
}

// Tests setting up initial presence of all active types, with some vaccinated 
bool create_active_w_vaccinated()
{
	double dt = 0.25;
	int inf0 = 10, N_active = 30000, N_vac = 10000;
	bool vaccinate = true;
	// Expected parameters
	int exp_num_vac = 1230;
	double exp_frac_flu = 0.113, exp_test_flu = 0.45;
	double exp_beta_sch_students = 0.4655, exp_beta_sch_emp = 0.231;
	double exp_beta_work = 0.3762, exp_beta_work_out = 0.00057; 
	double exp_beta_carpool = 0.513, exp_beta_transit = 0.015535;
	double exp_beta_leisure = 0.0399;	

	int num_vac = 0, num_flu = 0, inf0_count = 0;
	int num_flu_tested = 0.0; 

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
	// File with 	
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
	
	// Check the number of vaccinated
	if (abm.get_total_vaccinated() != N_vac + infection_parameters.at("initially vaccinated")) {
		std::cerr << "Mismatch in initially vaccinated agents" << std::endl;
		return false;
	} 

	// Counters related to activating COVID-19 cases
	std::map<std::string, int> state_counter = {
			 	{"Total exposed", 0}, {"Total symptomatic", 0},
			 	{"Exposed waiting for test", 0}, 
				{"Symptomatic waiting for test", 0},
				{"Exposed getting tested", 0}, {"Symptomatic getting tested", 0},
			 	{"Exposed waiting for results", 0}, 
				{"Symptomatic waiting for results", 0},
				{"Exposed home isolated", 0}, 
				{"Symptomatic home isolated", 0}};

	// These will be for agents that are also vaccinated
	std::map<std::string, int> vaccinated_state_counter = {
			 	{"Total exposed", 0}, {"Total symptomatic", 0},
			 	{"Exposed waiting for test", 0}, 
				{"Symptomatic waiting for test", 0},
				{"Exposed getting tested", 0}, {"Symptomatic getting tested", 0},
			 	{"Exposed waiting for results", 0}, 
				{"Symptomatic waiting for results", 0},
				{"Exposed home isolated", 0}, 
				{"Symptomatic home isolated", 0}};

	// Check relevant agent properties 
	for (const auto& agent : agents) { 
		const int aID = agent.get_ID();
		if (agent.vaccinated()) {
			++num_vac;
		}
		if (agent.symptomatic_non_covid()) {
			++num_flu;
			if (agent.tested()) {
				++num_flu_tested;
			} 
		} 		
		if (agent.infected()) {
			++inf0_count;
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
			} else {
				std::cerr << "Infected agent neither symptomatic nor exposed" << std::endl;
				return false;
			}
		}
	}

	// All states need to be larger than 0
	for (const auto& smap : state_counter) {
		if (smap.second == 0) {
			std::cerr << "Zero agents in state " << smap.first << std::endl;
			return false;
		}		
	}
	for (const auto& smap : vaccinated_state_counter) {
		if (smap.second == 0) {
			std::cerr << "Zero vaccinated agents in state " << smap.first << std::endl;
			return false;
		}		
	}
	if (static_cast<int>(infection_parameters.at("initially vaccinated")) + N_vac != num_vac) {
		std::cerr << "Initial number of vaccinated agents does not match expectations" << std::endl;
		return false;
	}
	if (num_flu == 0) {
		std::cerr << "No agents with flu" << std::endl;
		return false;
	}
	if (!check_fractions(num_flu_tested, num_flu, exp_test_flu, 
							"Wrong percentage of agents with flu that will be tested")){
		return false;
	}
	if (inf0 + N_active != inf0_count) {
		std::cerr << "Initially infected agent numbers don't match expectations" << std::endl;
		return false;
	}

	return true;
}

// Test suite for agents that are infected at intialization
bool check_initially_infected(const Agent& agent, const Flu& flu, int& n_exposed_never_sy,
								const std::map<std::string, double> infection_parameters)
{
	if (!agent.exposed() || agent.tested() || agent.home_isolated()){
		std::cerr << "Initially infected agent should be exposed and not tested" << std::endl;
		return false;
	}
	// Flu
	const int aID = agent.get_ID();
	const std::vector<int> flu_susceptible = flu.get_susceptible_IDs();
	for (const auto& afs : flu_susceptible){
		if (afs == aID){
			std::cerr << "Infected agent should not be part of future flu poll" << std::endl; 
			return false;
		}
	}
	const std::vector<int> flu_agents = flu.get_flu_IDs();
	for (const auto& afs : flu_agents){
		if (afs == aID){
			std::cerr << "Infected agent should not be part of flu group" << std::endl; 
			return false;
		}
	}
	if (agent.recovering_exposed()){
		++n_exposed_never_sy;
		// Recovery time 
		if (agent.get_latency_end_time() < infection_parameters.at("recovery time")){
			std::cerr << "Wrong latency of agent that will not develop symptoms" << std::endl; 
			return false;
		}
	}
	// Infectiousness variability
	if (agent.get_inf_variability_factor() < 0){
		std::cerr << "Agent's infectiousness variability out of range: " << agent.get_inf_variability_factor() << std::endl;
		return false;
	}
	// Latency and non-infectious period
	if ((agent.get_infectiousness_start_time() > agent.get_latency_end_time()) || 
			(agent.get_latency_end_time() < 0)){
		std::cerr << "Agent's non-infectious period or latency out of range" << std::endl;
		return false;
	}
	return true;
}

/// \brief Compare input places file with output from a Place object 
bool compare_places_files(std::string fname_in, std::string fname_out, 
				const std::vector<double> infection_parameters, 
				const bool has_type, const bool has_extra, const bool prints_type)
{
	std::ifstream input(fname_in);
	std::ifstream output(fname_out);

	// Load the first file as vectors
	std::vector<int> in_IDs;
	std::vector<double> in_coords_x, in_coords_y;

	int ID = 0;
	double x = 0.0, y = 0.0;
	std::string type = {}, out_type = {};
	std::vector<std::string> vec_types = {}; 
	int extra = 0;
	if (has_type == false){
		while (input >> ID >> x >> y){
			in_IDs.push_back(ID);
			in_coords_x.push_back(x);
			in_coords_y.push_back(y);
		}
	} else if (has_extra == false){
		// For places that have types
		while (input >> ID >> x >> y >> type){
			in_IDs.push_back(ID);
			in_coords_x.push_back(x);
			in_coords_y.push_back(y);
			vec_types.push_back(type);
		}
	} else if (has_extra == true){
		// For places that have types and an 
		// additional column 
		while (input >> ID >> x >> y >> type >> extra){
			in_IDs.push_back(ID);
			in_coords_x.push_back(x);
			in_coords_y.push_back(y);
			vec_types.push_back(type);
		}
	}

	// Now load the second (output) and compare
	// Also check if total number of agents and infected agents is 0
	int num_ag = 0, num_inf = 0;
	int num_ag_exp = 0, num_inf_exp = 0;
	int ind = 0;
	double parameter;

	while (output >> ID >> x >> y >> num_ag >> num_inf){
		// Compare ID, location, agents
		if (ID != in_IDs.at(ind)){
			std::cerr << "Wrong place ID" << std::endl;
			return false;
		}
		if (!float_equality<double>(x, in_coords_x.at(ind), 1e-5)){
			std::cerr << "Wrong x coordinates" << std::endl;
			return false;
		}
		if (!float_equality<double>(y, in_coords_y.at(ind), 1e-5)){
			std::cerr << "Wrong y coordinates" << std::endl;
			return false;
		}
		if (num_ag != num_ag_exp){
			std::cerr << "Wrong number of agents" << std::endl;
			return false;
		}
		if (num_inf != num_inf_exp){
			std::cerr << "Wrong number of infected agents" << std::endl;
			return false;
		}
		// Compare infection parameters
		for (auto const& expected_parameter : infection_parameters){
			output >> parameter;
			if (!float_equality<double>(expected_parameter, parameter, 1e-5)){
				std::cerr << "Wrong infection transmission parameter" << std::endl;
				return false;
			}
		}
		if ((prints_type == true) and (has_type == true)) {
			output >> out_type;
			if (out_type != vec_types.at(ind)) {
				std::cerr << "Object types do not match" << std::endl;
				return false;
			}		
		} else if ((prints_type == true) and (has_type == false)) {
			output >> out_type;
			// Place holder of sort
			if (out_type != "None") {
				std::cerr << "Object types do not match" << std::endl;
				return false;
			}
		}
		++ind;
	}

	// In case file empty, shorter, or doesn't exist
	if (in_IDs.size() != ind){
		std::cerr << "Wrong number of locations" << std::endl;
		return false;
	}
	return true;	
}

/// \brief Compare input workplace file with output from a Workplace object 
bool compare_workplaces_files(std::string fname_in, std::string fname_out, 
				const std::map<std::string, double>& infection_parameters)
{
	std::ifstream input(fname_in);
	std::ifstream output(fname_out);

	// Load the first file as vectors
	std::vector<int> in_IDs;
	std::vector<double> in_coords_x, in_coords_y;

	int ID = 0;
	double x = 0.0, y = 0.0;
	std::string type = {}, out_type = {};
	std::vector<std::string> vec_types = {}; 
	int extra = 0;
	// For places that have types and an 
	// additional column 
	while (input >> ID >> x >> y >> type >> extra){
		in_IDs.push_back(ID);
		in_coords_x.push_back(x);
		in_coords_y.push_back(y);
		vec_types.push_back(type);
	}

	// Now load the second (output) and compare
	// Also check if total number of agents and infected agents is 0
	int num_ag = 0, num_inf = 0;
	int num_ag_exp = 0, num_inf_exp = 0;
	int ind = 0;
	double parameter;

	while (output >> ID >> x >> y >> num_ag >> num_inf){
		// Compare ID, location, agents
		if (ID != in_IDs.at(ind)){
			std::cerr << "Wrong place ID" << std::endl;
			return false;
		}
		if (!float_equality<double>(x, in_coords_x.at(ind), 1e-5)){
			std::cerr << "Wrong x coordinates" << std::endl;
			return false;
		}
		if (!float_equality<double>(y, in_coords_y.at(ind), 1e-5)){
			std::cerr << "Wrong y coordinates" << std::endl;
			return false;
		}
		if (num_ag != num_ag_exp){
			std::cerr << "Wrong number of agents" << std::endl;
			return false;
		}
		if (num_inf != num_inf_exp){
			std::cerr << "Wrong number of infected agents" << std::endl;
			return false;
		}
		// Compare infection parameters
		// ck
		output >> parameter;
		if (!float_equality<double>(infection_parameters.at("severity correction"), parameter, 1e-5)){
			std::cerr << "Wrong severity correction" << std::endl;
			return false;
		}
		// beta
		output >> parameter;
		double occ_beta = 0;
		if (vec_types.at(ind) == "A") {
			occ_beta = infection_parameters.at("management science art transmission rate");
		} else if (vec_types.at(ind) == "B") {
			occ_beta = infection_parameters.at("service occupation transmission rate");
		} else if  (vec_types.at(ind) == "C") {
			occ_beta = infection_parameters.at("sales office transmission rate");
		} else if (vec_types.at(ind) == "D") {
			occ_beta = infection_parameters.at("construction maintenance transmission rate");
		} else if (vec_types.at(ind) == "E") {
			occ_beta = infection_parameters.at("production transportation transmission rate");
		} else if (vec_types.at(ind) == "outside") {
			occ_beta = 1.0;
		} else {
			std::cerr << "Unsupported workplace type " << vec_types.at(ind)<< std::endl;
			return false;
		}
		if (!float_equality<double>(occ_beta, parameter, 1e-5)){
			std::cerr << "Wrong transmission rate " << occ_beta << " " << parameter << std::endl;
			return false;
		}
		// psi
		output >> parameter;
		if (!float_equality<double>(infection_parameters.at("work absenteeism correction"), parameter, 1e-5)){
			std::cerr << "Wrong absenteism correction" << std::endl;
			return false;
		}
		output >> out_type;
		if (out_type != vec_types.at(ind)) {
			std::cerr << "Object types do not match" << std::endl;
			return false;
		}		
		++ind;
	}

	// In case file empty, shorter, or doesn't exist
	if (in_IDs.size() != ind){
		std::cerr << "Wrong number of locations" << std::endl;
		return false;
	}
	return true;	
}

/// \brief Compare input transit file with output from a Place/Transit object 
bool compare_transit_files(std::string fname_in, std::string fname_out, 
				const std::vector<double> infection_parameters)
{
	std::ifstream input(fname_in);
	std::ifstream output(fname_out);

	// Load the first file as vectors
	std::vector<int> in_IDs;
	double in_coords_x = 0.0, in_coords_y = 0.0;
	std::vector<std::string> in_types = {}; 

	int ID = 0;
	double x = 0.0, y = 0.0;
	std::string type = {}, out_type;
	double extra_1 = 0, extra_2 = 0;
	while (input >> ID >> type >> extra_1 >> extra_2){
			in_IDs.push_back(ID);
			in_types.push_back(type);
	}

	// Now load the second (output) and compare
	// Also check if total number of agents and infected agents is 0
	int num_ag = 0, num_inf = 0;
	int num_ag_exp = 0, num_inf_exp = 0;
	int ind = 0;
	double parameter;

	while (output >> ID >> x >> y >> num_ag >> num_inf){
		// Compare ID, location, agents
		if (ID != in_IDs.at(ind)){
			std::cerr << "Wrong place ID" << std::endl;
			return false;
		}
		if (!float_equality<double>(x, in_coords_x, 1e-5)){
			std::cerr << "Wrong x coordinates" << std::endl;
			return false;
		}
		if (!float_equality<double>(y, in_coords_y, 1e-5)){
			std::cerr << "Wrong y coordinates" << std::endl;
			return false;
		}
		if (num_ag != num_ag_exp){
			std::cerr << "Wrong number of agents" << std::endl;
			return false;
		}
		if (num_inf != num_inf_exp){
			std::cerr << "Wrong number of infected agents" << std::endl;
			return false;
		}
		// Compare infection parameters
		for (auto const& expected_parameter : infection_parameters){
			output >> parameter;
			if (!float_equality<double>(expected_parameter, parameter, 1e-5)){
				std::cerr << "Wrong infection transmission parameter" << std::endl;
				return false;
			}
		}
		// Type	
		output >> out_type;
		if (out_type != in_types.at(ind)) {
			std::cerr << "Object types do not match" << std::endl;
			return false;
		}		
		++ind;
	}

	// In case file empty, shorter, or doesn't exist
	if (in_IDs.size() != ind){
		std::cerr << "Wrong number of transit objects" << std::endl;
		return false;
	}
	return true;	
}

/// \brief Compare input agent file with output from an Agent object 
bool compare_agents_files(std::string fname_in, std::string fname_out)
{
	std::ifstream input(fname_in);
	std::ifstream output(fname_out);

	// Load the first file as vectors and generate IDs
	std::vector<int> IDs;
	// student, works, hospital staff, hospital patient
	std::vector<std::vector<bool>> status;
	std::vector<bool> infected;
	std::vector<int> age;
	std::vector<std::vector<double>> position;
	// house ID, school ID, work ID, hospital ID
	std::vector<std::vector<int>> places;		 
	
	int ID = 0, yrs = 0, hID = 0, sID = 0, wID = 0, hspID = 0;
	bool studies = false, works = false, is_infected = false;
	bool works_at_hospital = false, non_covid_patient = false;
	double x = 0.0, y = 0.0;

	while (input >> studies >> works >> yrs 
			>> x >> y >> hID >> non_covid_patient >> sID >> wID 
			>> works_at_hospital >> hspID >> is_infected)
	{
		IDs.push_back(++ID);
		status.push_back({studies, works, works_at_hospital, non_covid_patient});
		infected.push_back(is_infected);
		age.push_back(yrs);
		position.push_back({x, y});
		places.push_back({hID, sID, wID, hspID});
	}

	// Now load the second (output) file and compare
	int ind = 0;
	while (output >> ID >> studies >> works >> yrs 
			>> x >> y >> hID >> non_covid_patient >> sID >> wID 
			>> works_at_hospital >> hspID >> is_infected)
	{
		if (ID != IDs.at(ind))
			return false;
		if (studies != status.at(ind).at(0) 
				|| works != status.at(ind).at(1)
				|| works_at_hospital != status.at(ind).at(2)
				|| non_covid_patient != status.at(ind).at(3))
			return false;
		if (is_infected != infected.at(ind))
			return false;
		if (yrs != age.at(ind))
			return false;
		if (!float_equality<double>(x, position.at(ind).at(0), 1e-5))
			return false;
		if (!float_equality<double>(y, position.at(ind).at(1), 1e-5))
			return false;
		if (hID != places.at(ind).at(0) 
				|| sID != places.at(ind).at(1) 
				|| wID != places.at(ind).at(2)
				|| hspID != places.at(ind).at(3))
			return false;
		++ind;
	}

	// In case file empty, shorter, or doesn't exist
	//if (IDs.size() != ind)
	//	return false;
	return true;	
}

/**
 * \brief Check if agents registered correctly in a given Place
 *
 * @param abm - an ABM object
 * @param place_info - vector of expected total number of agents in each place and total infected
 * @param place_agents - vector with IDs of agents in each place
 * @param place_type - type of place (house, school or work), case independent
 * @param info_file - name of the file to save basic info output to 
 * @param agent_file - name of the file to save agent IDs to
 * @param num_red_args - number of redundant arguments (i.e. infection paramters) in printed
 */ 
bool correctly_registered(const ABM abm, const std::vector<std::vector<int>> place_info, 
							const std::vector<std::vector<int>> place_agents, std::string place_type, 
							const std::string info_file, const std::string agent_file, const int num_red_args)
{

	// Save basic info and agent IDs
	place_type = str_to_lower(place_type);
	if (place_type == "house"){
		abm.print_households(info_file);
		abm.print_agents_in_households(agent_file);
	} else if (place_type == "school"){
		abm.print_schools(info_file);
		abm.print_agents_in_schools(agent_file);
	} else if (place_type == "workplace"){
		abm.print_workplaces(info_file);
		abm.print_agents_in_workplaces(agent_file);
	} else if (place_type == "hospital"){
		abm.print_hospitals(info_file);
		abm.print_agents_in_hospitals(agent_file);
	} else{
		std::cout << "Wrong place type" << std::endl;
		return false; 
	}
	
	// Check if total number of agents and infected agents are correct
	std::ifstream info_total(info_file);
	int ID = 0, num_agents = 0, num_inf = 0;
	double x = 0.0, y = 0.0, not_needed_arg = 0.0;
	int ind = 0;
	while (info_total >> ID >> x >> y >> num_agents >> num_inf){
		// Ignore remaining 
		for (int i=0; i<num_red_args; ++i)
			info_total >> not_needed_arg;

		if (num_agents != place_info.at(ind).at(0))
			return false;
		if (num_inf != place_info.at(ind).at(1))
			return false;		
		++ind;
	}

	// Check if correct agent IDs
	// Load ID file into a nested vector, one inner vector per place
	std::vector<std::vector<int>> saved_IDs;
	std::ifstream info_IDs(agent_file);
	std::string line;
	while (std::getline(info_IDs, line))
	{	
		std::istringstream iss(line);
		std::vector<int> place_IDs;
		while (iss >> ID)
			place_IDs.push_back(ID);
		saved_IDs.push_back(place_IDs);
	}

	// Compare the vectors
	return is_equal_exact<int>(place_agents, saved_IDs);
}

/// Check if num1/num2 is roughly equal to expected
bool check_fractions(int num1, int num2, double fr_expected, std::string msg, bool need_ge)
{
	double fr_tested = static_cast<double>(num1)/static_cast<double>(num2);
	if (need_ge) {
		// Only need greater or equal 
		if (fr_tested < fr_expected){
			std::cout << msg << std::endl;
			std::cout << "Computed: " << fr_tested << " Expected: " << fr_expected << std::endl; 
			return false;
		}
	} else {
		if (!float_equality<double>(fr_tested, fr_expected, 0.1)){
			std::cout << msg << std::endl;
			std::cout << "Computed: " << fr_tested << " Expected: " << fr_expected << std::endl; 
			return false;
		}
	}
	return true;
}
