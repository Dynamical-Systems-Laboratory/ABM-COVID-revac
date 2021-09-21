#include <set>
#include <fstream>
#include "../../include/vaccinations.h"
#include "../../include/infection.h"
#include "../common/test_utils.h"

/***************************************************** 
 *
 * Test suite for the Vaccinations class 
 *
 *****************************************************/

using setter = void (Agent::*)(const bool);
using getter = bool (Agent::*)() const;
using nested_maps = const std::map<std::string, std::map<std::string, std::vector<std::vector<double>>>>;
using one_map = const std::map<std::string, std::vector<std::vector<double>>>;

// Tests
bool check_random_vaccinations_functionality();
bool check_group_vaccinations_functionality();
bool check_random_vaccinations_neg_time_offset();
bool check_random_revaccinations();

// Supporting functions
bool check_agent_vaccination_attributes(Agent& agent, const double time, 
											nested_maps&, const double offset, 
											const std::set<int>& revac_IDs = {}, std::ostream& out= std::cout);

int main()
{
	test_pass(check_random_vaccinations_functionality(), "Random vaccination functionality");
	test_pass(check_random_vaccinations_neg_time_offset(), "Random vaccination functionality - negative time offset");
	test_pass(check_group_vaccinations_functionality(), "Group vaccination functionality");
	test_pass(check_random_revaccinations(), "Random re-vaccination functionality");
}

bool check_random_vaccinations_functionality()
{
	// Vaccination settings
	const std::string data_dir("test_data/");
	const std::string fname("test_data/vaccination_parameters.txt");
	// Initial number to vaccinate
	int n_vac_0 = 10000;
	// Vaccinate this many each step
	int n_vac = 10;
	// Actual number of vaccinated
	int n_vac_cur = 0;	
	// Maximum number to vaccinate
	int n_vac_max = 30000;
	// Max number of time steps
	int n_steps = 150;
	// Probability of state assignements
	double prob = 0.0;
	// Time and time step
	double time = 0.0, dt = 0.25;
	// For random 
	Infection infection(dt);

	// Create agents
	// Number of agents
	int n_agents = 50000;
	// Default agent objects
	std::vector<Agent> agents(n_agents);
	// States some agents need to be in and their probabilities
	std::map<std::string, std::pair<double, setter>> agent_states = 
		{{"removed_dead", {0.1, &Agent::set_removed_dead}},
 		 {"tested_covid_positive", {0.25, &Agent::set_tested_covid_positive}},
		 {"removed_can_vaccinate", {0.53, &Agent::set_removed_can_vaccinate}},
		 {"former_suspected", {0.3, &Agent::set_former_suspected}},
		 {"symptomatic", {0.12, &Agent::set_symptomatic}},
		 {"symptomatic_non_covid", {0.18, &Agent::set_symptomatic_non_covid}},
		 {"home_isolated", {0.23, &Agent::set_home_isolated}},
		 {"needs_next_vaccination", {0.11, &Agent::set_needs_next_vaccination}}};	
	// Randomly assign age and different states
	int aID = 1;
	for (auto& agent : agents) {
		// Assign ID
		agent.set_ID(aID);	
		// Assign age
		agent.set_age(infection.get_int(0,100));
		// Assign other states
		prob = infection.get_uniform();
		for (const auto& state : agent_states) {
			if (prob <= state.second.first) {
				if (state.first == "removed_can_vaccinate") {
					agent.set_removed_recovered(true);
					if (infection.get_uniform() < 0.5) {
						(agent.*state.second.second)(false);
					} else {
						(agent.*state.second.second)(true);
					}
				} else {
					(agent.*state.second.second)(true);
				}
			} else {
				(agent.*state.second.second)(false);
			}	
		}
		++aID;			
	}

	// State checking 
	std::map<std::string, getter> agent_states_check = 
		{{"removed_dead", &Agent::removed_dead},
 		 {"tested_covid_positive", &Agent::tested_covid_positive},
		 {"removed_can_vaccinate", &Agent::removed_can_vaccinate},
		 {"former_suspected", &Agent::former_suspected},
		 {"symptomatic", &Agent::symptomatic},
		 {"symptomatic_non_covid", &Agent::symptomatic_non_covid},
		 {"home_isolated", &Agent::home_isolated},
		 {"needs_next_vaccination", &Agent::needs_next_vaccination}};

	// Vaccination functionality
	Vaccinations vaccinations(fname, data_dir);

	// Vaccination data
	nested_maps& vac_data_map = vaccinations.get_vaccination_data();
	// Vaccination parameters
	const std::map<std::string, double>& vac_params = vaccinations.get_vaccination_parameters();

	// Check max eligible
	int n_eligible = 0; 
	for (const auto& agent : agents) {
		bool not_eligible = false;
		if (agent.get_age() < vac_params.at("Minimum vaccination age")) {
			not_eligible = true;
		} else {
			for (const auto& state : agent_states_check) {
				if (state.first == "removed_can_vaccinate") {
					if ((agent.*state.second)() == false && agent.removed_recovered()) {
						not_eligible = true;
					}
				} else if ((agent.*state.second)() == true) {
					not_eligible = true;		
				}	
			}
		}
		if (not_eligible == false) {
			++n_eligible;				
		}		
	}
	// Should match max eligible at the begining 
	if (n_eligible != vaccinations.max_eligible_random(agents)) {
		std::cerr << "Wrong number of initially eligible to vaccinate" << std::endl;
		return false;
	}
	
	// Initially vaccinated
	n_vac_cur = vaccinations.vaccinate_random(agents, n_vac_0, infection, time);	
	
	// At this point they need to be the same (there is a sufficient number of agents)
	if (n_vac_cur != n_vac_0) {
		std::cerr << "Wrong number of initially vaccinated" << std::endl;
		return false;
	}	

	// Check properties with time
	std::vector<double> offsets(n_agents, 0.0);
	for (int i=0; i<n_steps; ++i) {
		for (auto& agent : agents) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map, offsets.at(agent.get_ID()-1))) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
				return false;		
			}
			// Update offsets of all non-vaccinated for new vaccinations
			// If they get vaccinated, they will have correct offsets
			if (!agent.vaccinated()) {
				offsets.at(agent.get_ID()-1) = time;
			}		
		}
		// Vaccinate new batch and save offsets
		n_vac_cur = vaccinations.vaccinate_random(agents, n_vac, infection, time);
		time += dt;
	}

	// Exceeding the limits
	n_eligible = vaccinations.max_eligible_random(agents);
	// Should vaccinate only n_eligible 
	n_vac = n_eligible + 1; 
	n_vac_cur = vaccinations.vaccinate_random(agents, n_vac, infection, time);
	if (n_vac_cur != n_eligible) {
		std::cerr << "Wrong number of agents vaccinated after the limit was exceeded" << std::endl;
		return false;
	}
	// Now it shouldn't vaccinate at all
	n_vac = 1;
	n_vac_cur = vaccinations.vaccinate_random(agents, n_vac, infection, time);
	if (n_vac_cur != 0) {
		std::cerr << "No agents should be vaccinated at this point" << std::endl;
		return false;
	}

	return true;
}

bool check_random_vaccinations_neg_time_offset()
{
	// Vaccination settings
	const std::string data_dir("test_data/");
	const std::string fname("test_data/vaccination_parameters.txt");
	// Initial number to vaccinate
	int n_vac_0 = 10000;
	// Vaccinate this many each step
	int n_vac = 10;
	// Actual number of vaccinated
	int n_vac_cur = 0;	
	// Maximum number to vaccinate
	int n_vac_max = 30000;
	// Max number of time steps
	int n_steps = 150;
	// Probability of state assignements
	double prob = 0.0;
	// Time and time step
	double time = 0.0, dt = 0.25;
	// For random 
	Infection infection(dt);

	// Create agents
	// Number of agents
	int n_agents = 50000;
	// Default agent objects
	std::vector<Agent> agents(n_agents);
	// States some agents need to be in and their probabilities
	std::map<std::string, std::pair<double, setter>> agent_states = 
		{{"removed_dead", {0.1, &Agent::set_removed_dead}},
 		 {"tested_covid_positive", {0.25, &Agent::set_tested_covid_positive}},
		 {"removed_can_vaccinate", {0.53, &Agent::set_removed_can_vaccinate}},
		 {"former_suspected", {0.3, &Agent::set_former_suspected}},
		 {"symptomatic", {0.12, &Agent::set_symptomatic}},
		 {"symptomatic_non_covid", {0.18, &Agent::set_symptomatic_non_covid}},
		 {"home_isolated", {0.23, &Agent::set_home_isolated}},
		 {"needs_next_vaccination", {0.11, &Agent::set_needs_next_vaccination}}};	
	// Randomly assign age and different states
	int aID = 1;
	for (auto& agent : agents) {
		// Assign ID
		agent.set_ID(aID);	
		// Assign age
		agent.set_age(infection.get_int(0,100));
		// Assign other states
		prob = infection.get_uniform();
		for (const auto& state : agent_states) {
			if (prob <= state.second.first) {
				if (state.first == "removed_can_vaccinate") {
					agent.set_removed_recovered(true);
					if (infection.get_uniform() < 0.5) {
						(agent.*state.second.second)(false);
					} else {
						(agent.*state.second.second)(true);
					}
				} else {
					(agent.*state.second.second)(true);
				}
			} else {
				(agent.*state.second.second)(false);
			}	
		}
		++aID;			
	}

	// State checking 
	std::map<std::string, getter> agent_states_check = 
		{{"removed_dead", &Agent::removed_dead},
 		 {"tested_covid_positive", &Agent::tested_covid_positive},
		 {"removed_can_vaccinate", &Agent::removed_can_vaccinate},
		 {"former_suspected", &Agent::former_suspected},
		 {"symptomatic", &Agent::symptomatic},
		 {"symptomatic_non_covid", &Agent::symptomatic_non_covid},
		 {"home_isolated", &Agent::home_isolated},
		 {"needs_next_vaccination", &Agent::needs_next_vaccination}};

	// Vaccination functionality
	Vaccinations vaccinations(fname, data_dir);

	// Vaccination data
	nested_maps& vac_data_map = vaccinations.get_vaccination_data();
	// Vaccination parameters
	const std::map<std::string, double>& vac_params = vaccinations.get_vaccination_parameters();

	// Check max eligible
	int n_eligible = 0; 
	for (const auto& agent : agents) {
		bool not_eligible = false;
		if (agent.get_age() < vac_params.at("Minimum vaccination age")) {
			not_eligible = true;
		} else {
			for (const auto& state : agent_states_check) {
				if (state.first == "removed_can_vaccinate") {
					if ((agent.*state.second)() == false && agent.removed_recovered()) {
						not_eligible = true;
					}
				} else if ((agent.*state.second)() == true) {
					not_eligible = true;		
				}	
			}
		}
		if (not_eligible == false) {
			++n_eligible;				
		}		
	}
	// Should match max eligible at the begining 
	if (n_eligible != vaccinations.max_eligible_random(agents)) {
		std::cerr << "Wrong number of initially eligible to vaccinate" << std::endl;
		return false;
	}
	
	// Initially vaccinated
	n_vac_cur = vaccinations.vaccinate_random_time_offset(agents, n_vac_0, infection, time);	
	
	// At this point they need to be the same (there is a sufficient number of agents)
	if (n_vac_cur != n_vac_0) {
		std::cerr << "Wrong number of initially vaccinated" << std::endl;
		return false;
	}	

	// Check properties with time
	for (int i=0; i<n_steps; ++i) {
		for (auto& agent : agents) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map, agent.get_vac_time_offset())) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
				return false;		
			}
		}
		// Vaccinate new batch and save offsets
		n_vac_cur = vaccinations.vaccinate_random_time_offset(agents, n_vac, infection, time);
		time += dt;
	}

	// Exceeding the limits
	n_eligible = vaccinations.max_eligible_random(agents);
	// Should vaccinate only n_eligible 
	n_vac = n_eligible + 1; 
	n_vac_cur = vaccinations.vaccinate_random_time_offset(agents, n_vac, infection, time);
	if (n_vac_cur != n_eligible) {
		std::cerr << "Wrong number of agents vaccinated after the limit was exceeded" << std::endl;
		return false;
	}
	// Now it shouldn't vaccinate at all
	n_vac = 1;
	n_vac_cur = vaccinations.vaccinate_random_time_offset(agents, n_vac, infection, time);
	if (n_vac_cur != 0) {
		std::cerr << "No agents should be vaccinated at this point" << std::endl;
		return false;
	}

	return true;
}

bool check_group_vaccinations_functionality()
{
	// Vaccination settings
	const std::string data_dir("test_data/");
	const std::string fname("test_data/vaccination_parameters.txt");
	// Group of choice
	std::string group_name("school employees");
	// Initial number to vaccinate
	int n_vac_0 = 100;
	// Vaccinate this many each step
	int n_vac = 10;
	// Actual number of vaccinated
	int n_vac_cur = 0;	
	// Maximum number to vaccinate
	int n_vac_max = 30000;
	// Max number of time steps
	int n_steps = 150;
	// Probability of state assignements
	double prob = 0.0;
	// Time and time step
	double time = 0.0, dt = 0.25;
	// For random 
	Infection infection(dt);

	// Create agents
	// Number of agents
	int n_agents = 50000;
	std::vector<Agent> agents;
	for (int i=0; i<n_agents; ++i) {
		// Create school employees or hospital employees
		if (infection.get_uniform() <= 0.7) {
			if (infection.get_uniform() <= 0.6) {
				// School employee
				Agent agent(false, false, 35, 0.0, 0.0, 1, false, 1, false, false,
							true, 1, false, 0, false, "walk", 10.0, 0, 0, false);
				agents.push_back(agent);
			} else {
				// Hospital employee
				Agent agent(false, false, 35, 0.0, 0.0, 1, false, 1, false, false,
							false, 1, true, 0, false, "walk", 10.0, 0, 0, false);
				agents.push_back(agent);
			}
		} else {
			// Default
			Agent agent;
			agents.push_back(agent);
		}
	}
	// States some agents need to be in and their probabilities
	std::map<std::string, std::pair<double, setter>> agent_states = 
		{{"removed_dead", {0.1, &Agent::set_removed_dead}},
 		 {"tested_covid_positive", {0.25, &Agent::set_tested_covid_positive}},
		 {"removed_can_vaccinate", {0.53, &Agent::set_removed_can_vaccinate}},
		 {"former_suspected", {0.3, &Agent::set_former_suspected}},
		 {"symptomatic", {0.12, &Agent::set_symptomatic}},
		 {"symptomatic_non_covid", {0.18, &Agent::set_symptomatic_non_covid}},
		 {"home_isolated", {0.23, &Agent::set_home_isolated}},
		 {"needs_next_vaccination", {0.11, &Agent::set_needs_next_vaccination}}};	
	// Randomly assign age and different states
	int aID = 1;
	for (auto& agent : agents) {
		// Assign ID
		agent.set_ID(aID);	
		// Assign age
		agent.set_age(infection.get_int(0,100));
		// Assign other states
		prob = infection.get_uniform();
		for (const auto& state : agent_states) {
			if (prob <= state.second.first) {
				if (state.first == "removed_can_vaccinate") {
					agent.set_removed_recovered(true);
					if (infection.get_uniform() < 0.5) {
						(agent.*state.second.second)(false);
					} else {
						(agent.*state.second.second)(true);
					}
				} else {
					(agent.*state.second.second)(true);
				}
			} else {
				(agent.*state.second.second)(false);
			}	
		}
		++aID;			
	}

	// State checking 
	std::map<std::string, getter> agent_states_check = 
		{{"removed_dead", &Agent::removed_dead},
 		 {"tested_covid_positive", &Agent::tested_covid_positive},
		 {"removed_can_vaccinate", &Agent::removed_can_vaccinate},
		 {"former_suspected", &Agent::former_suspected},
		 {"symptomatic", &Agent::symptomatic},
		 {"symptomatic_non_covid", &Agent::symptomatic_non_covid},
		 {"home_isolated", &Agent::home_isolated},
		 {"needs_next_vaccination", &Agent::needs_next_vaccination}};

	// Vaccination functionality
	Vaccinations vaccinations(fname, data_dir);

	// Vaccination data
	nested_maps& vac_data_map = vaccinations.get_vaccination_data();
	// Vaccination parameters
	const std::map<std::string, double>& vac_params = vaccinations.get_vaccination_parameters();

	// Check max eligible
	int n_eligible = 0; 
	for (const auto& agent : agents) {
		bool not_eligible = false;
		if (agent.get_age() < vac_params.at("Minimum vaccination age") 
				|| !agent.school_employee()) {
			not_eligible = true;
		} else {
			for (const auto& state : agent_states_check) {
				if (state.first == "removed_can_vaccinate") {
					if ((agent.*state.second)() == false && agent.removed_recovered()) {
						not_eligible = true;
					}
				} else if ((agent.*state.second)() == true) {
					not_eligible = true;		
				}	
			}
		}
		if (not_eligible == false) {
			++n_eligible;				
		}		
	}
	// Should match max eligible at the begining 
	if (n_eligible != vaccinations.max_eligible_group(agents, group_name)) {
		std::cerr << "Wrong number of initially eligible to vaccinate" << std::endl;
		return false;
	}
	
	// Initially vaccinated
	n_vac_cur = vaccinations.vaccinate_group(agents, group_name, n_vac_0, infection, time);	
	
	// At this point they need to be the same (there is a sufficient number of agents)
	if (n_vac_cur != n_vac_0) {
		std::cerr << "Wrong number of initially vaccinated" << std::endl;
		return false;
	}	

	// Check properties with time
	std::vector<double> offsets(n_agents, 0.0);
	for (int i=0; i<n_steps; ++i) {
		for (auto& agent : agents) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map, offsets.at(agent.get_ID()-1))) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
				return false;		
			}
			// Update offsets of all non-vaccinated for new vaccinations
			// If they get vaccinated, they will have correct offsets
			if (!agent.vaccinated()) {
				offsets.at(agent.get_ID()-1) = time;
			}		
		}
		// Vaccinate new batch and save offsets
		n_vac_cur = vaccinations.vaccinate_group(agents, group_name, n_vac, infection, time);
		// Vaccinate an entire group (just once)
		if (float_equality<double>(time, 10.0, 1e-5)) {
			n_vac_cur = vaccinations.vaccinate_group(agents, "hospital employees", n_vac, infection, time, true);
		}
		time += dt;
	}
	// Exceeding the limits
	n_eligible = vaccinations.max_eligible_group(agents, group_name);
	// Should vaccinate only n_eligible 
	n_vac = n_eligible + 1; 
	n_vac_cur = vaccinations.vaccinate_group(agents, group_name, n_vac, infection, time);
	if (n_vac_cur != n_eligible) {
		std::cerr << "Wrong number of agents vaccinated after the limit was exceeded" << std::endl;
		return false;
	}
	// Now it shouldn't vaccinate at all
	n_vac = 1;
	n_vac_cur = vaccinations.vaccinate_group(agents, group_name, n_vac, infection, time);
	if (n_vac_cur != 0) {
		std::cerr << "No agents should be vaccinated at this point" << std::endl;
		return false;
	}

	return true;
}

/// Tests all the states and properties related to vaccinations
bool check_agent_vaccination_attributes(Agent& agent, const double time, 
											nested_maps& vac_data_map, const double offset,
											const std::set<int>& revac_IDs, std::ostream& fout)
{
	double tol = 1e-3;
	if (!agent.vaccinated()) {
		// All vaccination-dependent properties should be at their default values
		if (!float_equality<double>(agent.vaccine_effectiveness(time), 0.0, tol)) {
			std::cerr << "Computed effectiveness not equal default." << std::endl;
			return false;	
		}
		if (!float_equality<double>(agent.asymptomatic_correction(time), 1.0, tol)) {
			std::cerr << "Computed probability of being asymptomatic not equal default." << std::endl;
			return false;	
		}
		if (!float_equality<double>(agent.transmission_correction(time), 1.0, tol)) {
			std::cerr << "Computed probability correction of transmission not equal default." << std::endl;
			return false;	
		}
		if (!float_equality<double>(agent.severe_correction(time), 1.0, tol)) {
			std::cerr << "Computed probability correction of developing severe disease not equal default." << std::endl;
			return false;	
		}
		if (!float_equality<double>(agent.death_correction(time), 1.0, tol)) {
			std::cerr << "Computed probability correction of dying not equal default." << std::endl;
			return false;	
		}
		if (!float_equality<double>(agent.get_time_vaccine_effects_reduction(), 0.0, tol)) {
			std::cerr << "Vaccine effects drop should be set to initial value (0.0) at this point" << std::endl;
			return false;	
		}
		if (!float_equality<double>(agent.get_time_mobility_increase(), 0.0, tol)) {
			std::cerr << "Time when mobility increases should be set to initial value (0.0) at this point" << std::endl;
			return false;	
		}
	} else {
		// For re-vaccinations - just print and inspect
		if ((!revac_IDs.empty()) && (revac_IDs.find(agent.get_ID())!=revac_IDs.end())) {
			fout << time << " " << agent.get_ID() << " " << agent.get_vaccine_subtype() << " " 
				 << agent.vaccine_effectiveness(time) << " " << agent.asymptomatic_correction(time) << " " 
				 << agent.transmission_correction(time) << " " 
				 << agent.severe_correction(time) << " " << agent.death_correction(time) << std::endl;
			return true; 
		}
		// Get the property map for this agents tag
		std::string tag = agent.get_vaccine_subtype();
		one_map& prop_map = vac_data_map.at(tag);
		// Construct tpf or fpf for each property
		// Check for current time if all properties equal expeceted
		if (agent.get_vaccine_type() == "one_dose") { 
			ThreePartFunction tpf_eff(prop_map.at("effectiveness"), offset);
			ThreePartFunction tpf_asm(prop_map.at("asymptomatic"), offset);
			ThreePartFunction tpf_tra(prop_map.at("transmission"), offset);
			ThreePartFunction tpf_sev(prop_map.at("severe"), offset);
			ThreePartFunction tpf_dth(prop_map.at("death"), offset);
			
			if (!float_equality<double>(agent.vaccine_effectiveness(time), tpf_eff(time), tol)) {
				std::cerr << "Computed effectiveness not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.asymptomatic_correction(time), tpf_asm(time), tol)) {
				std::cerr << "Computed probability of being asymptomatic not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.transmission_correction(time), tpf_tra(time), tol)) {
				std::cerr << "Computed probability correction of transmission not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.severe_correction(time), tpf_sev(time), tol)) {
				std::cerr << "Computed probability correction of developing severe disease not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.death_correction(time), tpf_dth(time), tol)) {
				std::cerr << "Computed probability correction of dying not equal expected" << std::endl;
				return false;	
			} 
		} else {
		 	FourPartFunction fpf_eff(prop_map.at("effectiveness"), offset);
			FourPartFunction fpf_asm(prop_map.at("asymptomatic"), offset);
			FourPartFunction fpf_tra(prop_map.at("transmission"), offset);
			FourPartFunction fpf_sev(prop_map.at("severe"), offset);
			FourPartFunction fpf_dth(prop_map.at("death"), offset);
	
			if (!float_equality<double>(agent.vaccine_effectiveness(time), fpf_eff(time), tol)) {
				std::cerr << "Computed effectiveness not equal expected " << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.asymptomatic_correction(time), fpf_asm(time), tol)) {
				std::cerr << "Computed probability of being asymptomatic not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.transmission_correction(time), fpf_tra(time), tol)) {
				std::cerr << "Computed probability correction of transmission not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.severe_correction(time), fpf_sev(time), tol)) {
				std::cerr << "Computed probability correction of developing severe disease not equal expected" << std::endl;
				return false;	
			}
			if (!float_equality<double>(agent.death_correction(time), fpf_dth(time), tol)) {
				std::cerr << "Computed probability correction of dying not equal expected" << std::endl;
				return false;	
			}
		}
		// General properties
		if (agent.get_time_vaccine_effects_reduction() < 0.0 && offset >= 0) {
			std::cerr << "Vaccine effects drop not set after vaccination" << std::endl;
			return false;	
		}
		if (agent.get_time_mobility_increase() < 0.0 && offset >= 0) {
			std::cerr << "Time when agent's mobility increases not set after vaccination" << std::endl;
			return false;	
		}
	}
	return true;
}

bool check_random_revaccinations()
{
	// Vaccination settings
	const std::string data_dir("test_data/");
	const std::string fname("test_data/vaccination_parameters.txt");
	// Initial number to vaccinate
	int n_vac_0 = 10000;
	// Vaccinate this many each step
	int n_vac = 10;
	// Actual number of vaccinated
	int n_vac_cur = 0;	
	// Maximum number to vaccinate
	int n_vac_max = 30000;
	// Max number of time steps
	int n_steps = 150;
	// Probability of state assignements
	double prob = 0.0;
	// Time and time step
	double time = 0.0, dt = 0.25;
	// For random 
	Infection infection(dt);
	// For saving properties of re-vaccinated
	std::ofstream fout("test_data/revac_stats.txt");

	// Create agents
	// Number of agents
	int n_agents = 50000;
	// Default agent objects
	std::vector<Agent> agents(n_agents);
	// States some agents need to be in and their probabilities
	std::map<std::string, std::pair<double, setter>> agent_states = 
		{{"removed_dead", {0.1, &Agent::set_removed_dead}},
 		 {"tested_covid_positive", {0.25, &Agent::set_tested_covid_positive}},
		 {"removed_can_vaccinate", {0.53, &Agent::set_removed_can_vaccinate}},
		 {"former_suspected", {0.3, &Agent::set_former_suspected}},
		 {"symptomatic", {0.12, &Agent::set_symptomatic}},
		 {"symptomatic_non_covid", {0.18, &Agent::set_symptomatic_non_covid}},
		 {"home_isolated", {0.23, &Agent::set_home_isolated}},
		 {"needs_next_vaccination", {0.71, &Agent::set_needs_next_vaccination}}};	

	// Vaccination functionality
	Vaccinations vaccinations(fname, data_dir);

	// Randomly assign age and different states
	int aID = 1;
	// Initially re-vaccinated
	std::set<int> revaccinated;
	for (auto& agent : agents) {
		// Assign ID
		agent.set_ID(aID);	
		// Assign age
		agent.set_age(infection.get_int(0,100));
		// Assign other states
		prob = infection.get_uniform();
		for (const auto& state : agent_states) {
			if (prob <= state.second.first) {
				if (state.first == "removed_can_vaccinate") {
					agent.set_removed_recovered(true);
					if (infection.get_uniform() < 0.5) {
						(agent.*state.second.second)(false);
					} else {
						(agent.*state.second.second)(true);
					}
				} else if (state.first == "needs_next_vaccination") {
					// Vaccinate just one 
					vaccinations.vaccinate_and_setup_time_offset(agents, 
						{agent.get_ID()}, infection, time);
					// Then reset these	
					(agent.*state.second.second)(true);
					agent.set_vaccinated(true);
					revaccinated.emplace(agent.get_ID());
					// And re-vaccinate
					// Check for 0.0 and non-zero (here 10) time
					vaccinations.vaccinate_and_setup_time_offset(agents, 
						{agent.get_ID()}, infection, time+10.0);	
					++n_vac_max;
				} else {
					(agent.*state.second.second)(true);
				}
			} else {
				(agent.*state.second.second)(false);
			}	
		}
		++aID;			
	}

	// State checking 
	std::map<std::string, getter> agent_states_check = 
		{{"removed_dead", &Agent::removed_dead},
 		 {"tested_covid_positive", &Agent::tested_covid_positive},
		 {"removed_can_vaccinate", &Agent::removed_can_vaccinate},
		 {"former_suspected", &Agent::former_suspected},
		 {"symptomatic", &Agent::symptomatic},
		 {"symptomatic_non_covid", &Agent::symptomatic_non_covid},
		 {"home_isolated", &Agent::home_isolated},
		 {"needs_next_vaccination", &Agent::needs_next_vaccination}};

	// Vaccination data
	nested_maps& vac_data_map = vaccinations.get_vaccination_data();
	// Vaccination parameters
	const std::map<std::string, double>& vac_params = vaccinations.get_vaccination_parameters();

	// Check max eligible
	int n_eligible = vaccinations.max_eligible_random(agents); 
	// Initially vaccinated
	n_vac_cur = vaccinations.vaccinate_random_time_offset(agents, n_vac_0, infection, time);	

	// At this point they need to be the same (there is a sufficient number of agents)
	if (n_vac_cur != n_vac_0) {
		std::cerr << "Wrong number of initially vaccinated" << std::endl;
		return false;
	}	
	
	// Check properties with time
	for (int i=0; i<n_steps; ++i) {
		for (auto& agent : agents) {
			if (!check_agent_vaccination_attributes(agent, time, vac_data_map, 
					agent.get_vac_time_offset(), revaccinated, fout)) {
				std::cerr << "Error in properties of vaccinated and not vaccinated agents" << std::endl;
				return false;		
			}
		}
		// Vaccinate new batch and save offsets
		n_vac_cur = vaccinations.vaccinate_random_time_offset(agents, n_vac, infection, time);
		time += dt;
	}

	// Exceeding the limits
	n_eligible = vaccinations.max_eligible_random(agents);
	// Should vaccinate only n_eligible 
	n_vac = n_eligible + 1; 
	n_vac_cur = vaccinations.vaccinate_random_time_offset(agents, n_vac, infection, time);
	if (n_vac_cur != n_eligible) {
		std::cerr << "Wrong number of agents vaccinated after the limit was exceeded" << std::endl;
		return false;
	}
	return true;
}
