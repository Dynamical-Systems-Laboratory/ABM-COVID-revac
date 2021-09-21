#include "../include/vaccinations.h"

// Load parameters related to vaccinations store in a map
void Vaccinations::load_vaccination_parameters(const std::string& infile, const std::string& data_dir)
{
	// Collect and store the parameters
	LoadParameters ldparam;
	vaccination_parameters = ldparam.load_parameter_map<double>(infile);

	// Loading and storing of time, value pairs for creating the time dependencies
	// All the one dose types 
	int num_one_dose = static_cast<int>(vaccination_parameters.at("Number of one dose types"));
	for (int i=1; i<=num_one_dose; ++i) {
		std::string file_name = data_dir + "one_dose_vac_type_" + std::to_string(i) + ".txt";
		std::string tag = "one dose - type "+ std::to_string(i);
		vac_types_properties[tag] = ldparam.load_table(file_name);
		vac_types_probs["one dose CDF"].push_back(vaccination_parameters.at(tag + " probability vaccinated, CDF"));		
	}
	// All the two dose types
	int num_two_dose = static_cast<int>(vaccination_parameters.at("Number of two dose types"));
	for (int i=1; i<=num_two_dose; ++i) {
		std::string file_name = data_dir + "two_dose_vac_type_" + std::to_string(i) + ".txt";
		std::string tag = "two dose - type "+ std::to_string(i);
		vac_types_properties[tag] = ldparam.load_table(file_name);
		vac_types_probs["two dose CDF"].push_back(vaccination_parameters.at(tag + " probability vaccinated, CDF"));		
	}
}

// Copy parameters in v1 into v2
void Vaccinations::copy_vaccination_dependencies(std::forward_list<double>&& lst,
									std::vector<std::vector<double>>& vec)
{
	std::vector<double> temp(2, 0.0);
	while (!lst.empty()) {
		// Collect point pairs (time and value)
		temp.at(0) = lst.front();
		lst.pop_front();
		temp.at(1) = lst.front();
		lst.pop_front();
		// Store the pair in the main vector
		vec.push_back(temp);
	}
}

// Randomly vaccinates requested number of agents
int Vaccinations::vaccinate_random(std::vector<Agent>& agents, int n_vac, 
							Infection& infection, const double time)
{
	// Pick ones that can be vaccinated
	std::vector<int> can_be_vaccinated = filter_general(agents);
	if (can_be_vaccinated.empty()) {
		std::cout << "No more agents eligible for random vaccination" << std::endl;
		return 0;
	}
	// Reduce n_vac if larger than available
	if (n_vac > can_be_vaccinated.size()) {
		n_vac = can_be_vaccinated.size();
		std::cout << "Requested number of agents for random vaccination"
				  << " larger than currently eligible -- decreasing to " 
				  << n_vac << std::endl;
	}
	// If not processing all avaiblable,
	// randomly shuffle the indices, then vaccinate first n_vac
	if (n_vac != can_be_vaccinated.size()) {
		infection.vector_shuffle(can_be_vaccinated);
		can_be_vaccinated.resize(n_vac);		
	}	
	// Vaccinate and set agent properties
	vaccinate_and_setup(agents, can_be_vaccinated, infection, time);
	return n_vac;
}

// Randomly vaccinates requested number of agents with a negative time offset
int Vaccinations::vaccinate_random_time_offset(std::vector<Agent>& agents, int n_vac, 
							Infection& infection, const double time)
{
	// Pick ones that can be vaccinated
	std::vector<int> can_be_vaccinated = filter_general(agents);
	if (can_be_vaccinated.empty()) {
		std::cout << "No more agents eligible for random vaccination" << std::endl;
		return 0;
	}
	// Reduce n_vac if larger than available
	if (n_vac > can_be_vaccinated.size()) {
		n_vac = can_be_vaccinated.size();
		std::cout << "Requested number of agents for random vaccination"
				  << " larger than currently eligible -- decreasing to " 
				  << n_vac << std::endl;
	}
	// If not processing all avaiblable,
	// randomly shuffle the indices, then vaccinate first n_vac
	if (n_vac != can_be_vaccinated.size()) {
		infection.vector_shuffle(can_be_vaccinated);
		can_be_vaccinated.resize(n_vac);		
	}	
	// Vaccinate and set agent properties
	vaccinate_and_setup_time_offset(agents, can_be_vaccinated, infection, time);
	return n_vac;
}

// Randomly vaccinates requested number of agents
int Vaccinations::vaccinate_group(std::vector<Agent>& agents, const std::string& group_name,
									int n_vac, Infection& infection, const double time,
									const bool vaccinate_all)
{
	// Pick ones that can be vaccinated
	std::vector<int> can_be_vaccinated = filter_general_and_group(agents, group_name);
	if (can_be_vaccinated.empty()) {
		std::cout << "No more agents eligible for vaccination of group " 
				  << group_name << std::endl;
		return 0;
	}
	// Reduce n_vac if larger than available
	if (n_vac > can_be_vaccinated.size()) {
		n_vac = can_be_vaccinated.size();
		std::cout << "Requested number of agents for vaccination of group "
				  << group_name 
				  << " larger than currently eligible -- decreasing to " 
				  << n_vac << std::endl;
	} else if (vaccinate_all){
		// If all are requested to be vaccinated
		n_vac =  can_be_vaccinated.size();
		std::cout << "Vaccinating all " << n_vac << " eligible agents in group "
				  << group_name << std::endl;
	}
	// If not processing all avaiblable,
	// randomly shuffle the indices, then vaccinate first n_vac
	if (n_vac != can_be_vaccinated.size()) {
		infection.vector_shuffle(can_be_vaccinated);
		can_be_vaccinated.resize(n_vac);		
	}	
	// Vaccinate and set agent properties
	vaccinate_and_setup(agents, can_be_vaccinated, infection, time);
	return n_vac;
}

// Returns maximum number of agents currently eligible for vaccination
int Vaccinations::max_eligible_random(const std::vector<Agent>& agents)
{
	std::vector<int> eligible_IDs = filter_general(agents);
	return eligible_IDs.size();
}

// Returns maximum number of agents in a group currently eligible for vaccination
int Vaccinations::max_eligible_group(const std::vector<Agent>& agents, const std::string& group_name)
{
	std::vector<int> eligible_IDs = filter_general_and_group(agents, group_name);
	return eligible_IDs.size();
}

// Select agents eligible for vaccination based on criteria valid for all agents
std::vector<int> Vaccinations::filter_general(const std::vector<Agent>& agents)
{
	std::vector<int> eligible_agents;
	for (const auto& agent : agents) {
		// Verify if the agent meets all the core requirements
		if (check_general(agent)) {
			eligible_agents.push_back(agent.get_ID());
		} 
	}
	return eligible_agents;	
}

// Select agents in a given group eligible for vaccination based on criteria valid for all agents
std::vector<int> Vaccinations::filter_general_and_group(const std::vector<Agent>& agents, 
									const std::string& group_name)
{
	std::vector<int> eligible_agents;
	for (const auto& agent : agents) {
		// First check if the agent belongs to the target group
		if (check_group(agent, group_name)) {
			// Then verify if the agent meets all the core requirements
			if (check_general(agent)) {
				eligible_agents.push_back(agent.get_ID());
			} 
		}
	}
	return eligible_agents;
}

// True if agent meets core criteria for vaccination eligibility
bool Vaccinations::check_general(const Agent& agent)
{
	if (agent.vaccinated() && !agent.needs_next_vaccination()) {
		return false;
	}
	if (agent.removed_dead()) {
		return false;
	} 
	if (agent.get_age() < vaccination_parameters.at("Minimum vaccination age")) {
		return false;	
	}
	if (agent.tested_covid_positive()) {
		return false; 
	}
	if (agent.removed_recovered() && !agent.removed_can_vaccinate()) {
		return false;
	}
 	if (agent.former_suspected() && !agent.suspected_can_vaccinate()) {
		return false;
	}
	if (agent.symptomatic()) {
		return false;
	}
	if (agent.symptomatic_non_covid()) {
		return false;
	}
	if (agent.home_isolated()) {
		return false;
	}
	if (agent.contact_traced()) {
		return false; 
	}
	return true;
}

// True if agent is in the target vaccination group 
bool Vaccinations::check_group(const Agent& agent, const std::string& vaccine_group_name)
{
	if (vaccine_group_name == "hospital employees"
			&& agent.hospital_employee()) {
		return true;
	} else if (vaccine_group_name == "school employees"
			&& agent.school_employee()) {
		return true;
	} else if (vaccine_group_name == "retirement home employees"
			&& agent.retirement_home_employee()) {
		return true;
	} else if (vaccine_group_name == "retirement home residents"
			&& agent.retirement_home_resident()) {
		return true;
	} 	
	return false;
}

// Vaccinates agents with provided IDs and sets all the agent properties
void Vaccinations::vaccinate_and_setup(std::vector<Agent>& agents, const std::vector<int>& agent_IDs, 
										Infection& infection, const double time)
{
	for (auto& id : agent_IDs) {
		Agent& agent = agents.at(id-1);
		// Third dose
		if (agent.vaccinated() && agent.needs_next_vaccination()) {
			double next_step = vaccination_parameters.at("Third dose max effects time");
			double max_end = vaccination_parameters.at("Third dose max effects end time");
			double tot_end = vaccination_parameters.at("Third dose no effects time");
			std::string vac_type = agent.get_vaccine_type();
			std::string tag = agent.get_vaccine_subtype();
			// Construct for each benefit: this step, current value | next step, max value | then as usual
			// 1) Effectiveness
			std::vector<std::vector<double>> orig_props = vac_types_properties.at(tag).at("effectiveness");
			double max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_eff{{0.0, agent.vaccine_effectiveness(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_vaccine_effectiveness(ThreePartFunction(new_eff, time));
			// 2) Asymptomatic correction
			orig_props = vac_types_properties.at(tag).at("asymptomatic");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_asm{{0.0, agent.asymptomatic_correction(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_asymptomatic_correction(ThreePartFunction(new_asm, time));
			// 3) Transmission correction
			orig_props = vac_types_properties.at(tag).at("transmission");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_tr{{0.0, agent.transmission_correction(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_transmission_correction(ThreePartFunction(new_tr, time));	
			// 4) Severity correction
			orig_props = vac_types_properties.at(tag).at("severe");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_sv{{0.0, agent.severe_correction(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_severe_correction(ThreePartFunction(new_sv, time));
			// 5) Death correction
			orig_props = vac_types_properties.at(tag).at("death");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_dth{{0.0, agent.death_correction(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_death_correction(ThreePartFunction(new_dth, time));
			// Other properties
			// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
			agent.set_time_vaccine_effects_reduction(time+max_end);
			// and the time when mobility increases (at peak effectiveness)
			agent.set_time_mobility_increase(time);
			// To not keep on vaccinating
			agent.set_needs_next_vaccination(false);
			// Correct the type
			agent.set_vaccine_type("one_dose");
			agent.set_vaccine_subtype("former " + tag);
			continue;
		}

		// First vaccination ever
		agent.set_vaccinated(true);
		agent.set_needs_next_vaccination(false);
		if ( infection.get_uniform() <= vaccination_parameters.at("Fraction taking one dose vaccine")) {
			agent.set_vaccine_type("one_dose");
			// Select the type based on the iterator in the CDF
			// This assumes all types are loaded sequentially
			std::vector<double> one_dose_probs = vac_types_probs.at("one dose CDF");
			double cur_prob = infection.get_uniform();
			const auto& iter = std::find_if(one_dose_probs.cbegin(), one_dose_probs.cend(), 
					[&cur_prob](const double x) { return x >= cur_prob; });
			// Types start with 1
			std::string tag = "one dose - type "+ std::to_string(std::distance(one_dose_probs.cbegin(), iter) + 1);
			agent.set_vaccine_subtype(tag);
			// Now access attributes for the three part functions and set-up agent properties		
			agent.set_vaccine_effectiveness(ThreePartFunction(vac_types_properties.at(tag).at("effectiveness"), time));
			agent.set_asymptomatic_correction(ThreePartFunction(vac_types_properties.at(tag).at("asymptomatic"), time));
			agent.set_transmission_correction(ThreePartFunction(vac_types_properties.at(tag).at("transmission"), time));
			agent.set_severe_correction(ThreePartFunction(vac_types_properties.at(tag).at("severe"), time));
			agent.set_death_correction(ThreePartFunction(vac_types_properties.at(tag).at("death"), time));
			// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
			agent.set_time_vaccine_effects_reduction(time+vac_types_properties.at(tag).at("effectiveness").at(2).at(0));
			// and the time when mobility increases (at peak effectiveness)
			agent.set_time_mobility_increase(time+vac_types_properties.at(tag).at("effectiveness").at(1).at(0));
		} else {
			agent.set_vaccine_type("two_doses");
			// Select the type based on the iterator in the CDF
			// This assumes all types are loaded sequentially
			std::vector<double> two_dose_probs = vac_types_probs.at("two dose CDF");
			double cur_prob = infection.get_uniform();
			const auto& iter = std::find_if(two_dose_probs.cbegin(), two_dose_probs.cend(), 
					[&cur_prob](const double x) { return x >= cur_prob; });
			// Types start with 1
			std::string tag = "two dose - type "+ std::to_string(std::distance(two_dose_probs.cbegin(), iter) + 1);
			agent.set_vaccine_subtype(tag);
			// Now access attributes for the four part functions and set-up agent properties		
			agent.set_vaccine_effectiveness(FourPartFunction(vac_types_properties.at(tag).at("effectiveness"), time));
			agent.set_asymptomatic_correction(FourPartFunction(vac_types_properties.at(tag).at("asymptomatic"), time));
			agent.set_transmission_correction(FourPartFunction(vac_types_properties.at(tag).at("transmission"), time));
			agent.set_severe_correction(FourPartFunction(vac_types_properties.at(tag).at("severe"), time));
			agent.set_death_correction(FourPartFunction(vac_types_properties.at(tag).at("death"), time));
			// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
			agent.set_time_vaccine_effects_reduction(time+vac_types_properties.at(tag).at("effectiveness").at(3).at(0));
			// and the time when mobility increases (at peak effectiveness)
			agent.set_time_mobility_increase(time+vac_types_properties.at(tag).at("effectiveness").at(2).at(0));
		}
	}
}

// Vaccinates agents with provided IDs and sets all the agent properties while applying a negative time offset
void Vaccinations::vaccinate_and_setup_time_offset(std::vector<Agent>& agents, const std::vector<int>& agent_IDs, 
										Infection& infection, const double time)
{
	const double t0 = vaccination_parameters.at("Start of time offset interval");
	const double tf = vaccination_parameters.at("End of time offset interval");
	double offset = 0;
	for (auto& id : agent_IDs) {
		Agent& agent = agents.at(id-1);
		// Third dose
		if (agent.vaccinated() && agent.needs_next_vaccination()) {
			double next_step = vaccination_parameters.at("Third dose max effects time");
			double max_end = vaccination_parameters.at("Third dose max effects end time");
			double tot_end = vaccination_parameters.at("Third dose no effects time");
			std::string vac_type = agent.get_vaccine_type();
			std::string tag = agent.get_vaccine_subtype();
			// Construct for each benefit: this step, current value | next step, max value | then as usual
			// 1) Effectiveness
			std::vector<std::vector<double>> orig_props = vac_types_properties.at(tag).at("effectiveness");
			double max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_eff{{0.0, agent.vaccine_effectiveness(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_vaccine_effectiveness(ThreePartFunction(new_eff, time));
			// 2) Asymptomatic correction
			orig_props = vac_types_properties.at(tag).at("asymptomatic");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_asm{{0.0, agent.asymptomatic_correction(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_asymptomatic_correction(ThreePartFunction(new_asm, time));
			// 3) Transmission correction
			orig_props = vac_types_properties.at(tag).at("transmission");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_tr{{0.0, agent.transmission_correction(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_transmission_correction(ThreePartFunction(new_tr, time));	
			// 4) Severity correction
			orig_props = vac_types_properties.at(tag).at("severe");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_sv{{0.0, agent.severe_correction(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_severe_correction(ThreePartFunction(new_sv, time));
			// 5) Death correction
			orig_props = vac_types_properties.at(tag).at("death");
			max_benefit = orig_props.at(orig_props.size()-2).at(1);
			std::vector<std::vector<double>> new_dth{{0.0, agent.death_correction(time)}, 
												{next_step, max_benefit}, {max_end, max_benefit}, {tot_end, 0.0}};
			agent.set_death_correction(ThreePartFunction(new_dth, time));
			// Other properties
			// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
			agent.set_time_vaccine_effects_reduction(time+max_end);
			// and the time when mobility increases (at peak effectiveness)
			agent.set_time_mobility_increase(time);
			// To not keep on vaccinating
			agent.set_needs_next_vaccination(false);
			// Correct the type 
			agent.set_vaccine_type("one_dose");
			agent.set_vaccine_subtype("former " + tag);
			continue;
		}

		// First vaccination ever
		agent.set_vaccinated(true);
		agent.set_needs_next_vaccination(false);
		// This amount of time will be subtracted from the current time
		offset = -1.0*infection.get_uniform(t0, tf);
		agent.set_vac_time_offset(offset);
		if ( infection.get_uniform() <= vaccination_parameters.at("Fraction taking one dose vaccine")) {
			agent.set_vaccine_type("one_dose");
			// Select the type based on the iterator in the CDF
			// This assumes all types are loaded sequentially
			std::vector<double> one_dose_probs = vac_types_probs.at("one dose CDF");
			double cur_prob = infection.get_uniform();
			const auto& iter = std::find_if(one_dose_probs.cbegin(), one_dose_probs.cend(), 
					[&cur_prob](const double x) { return x >= cur_prob; });
			// Types start with 1
			std::string tag = "one dose - type "+ std::to_string(std::distance(one_dose_probs.cbegin(), iter) + 1);
			agent.set_vaccine_subtype(tag);
			// Now access attributes for the three part functions and set-up agent properties		
			agent.set_vaccine_effectiveness(ThreePartFunction(vac_types_properties.at(tag).at("effectiveness"), offset));
			agent.set_asymptomatic_correction(ThreePartFunction(vac_types_properties.at(tag).at("asymptomatic"), offset));
			agent.set_transmission_correction(ThreePartFunction(vac_types_properties.at(tag).at("transmission"), offset));
			agent.set_severe_correction(ThreePartFunction(vac_types_properties.at(tag).at("severe"), offset));
			agent.set_death_correction(ThreePartFunction(vac_types_properties.at(tag).at("death"), offset));
			// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
			agent.set_time_vaccine_effects_reduction(offset+vac_types_properties.at(tag).at("effectiveness").at(2).at(0));
			// and the time when mobility increases (at peak effectiveness)
			agent.set_time_mobility_increase(offset+vac_types_properties.at(tag).at("effectiveness").at(1).at(0));
		} else {
			agent.set_vaccine_type("two_doses");
			// Select the type based on the iterator in the CDF
			// This assumes all types are loaded sequentially
			std::vector<double> two_dose_probs = vac_types_probs.at("two dose CDF");
			double cur_prob = infection.get_uniform();
			const auto& iter = std::find_if(two_dose_probs.cbegin(), two_dose_probs.cend(), 
					[&cur_prob](const double x) { return x >= cur_prob; });
			// Types start with 1
			std::string tag = "two dose - type "+ std::to_string(std::distance(two_dose_probs.cbegin(), iter) + 1);
			agent.set_vaccine_subtype(tag);
			// Now access attributes for the four part functions and set-up agent properties		
			agent.set_vaccine_effectiveness(FourPartFunction(vac_types_properties.at(tag).at("effectiveness"), offset));
			agent.set_asymptomatic_correction(FourPartFunction(vac_types_properties.at(tag).at("asymptomatic"), offset));
			agent.set_transmission_correction(FourPartFunction(vac_types_properties.at(tag).at("transmission"), offset));
			agent.set_severe_correction(FourPartFunction(vac_types_properties.at(tag).at("severe"), offset));
			agent.set_death_correction(FourPartFunction(vac_types_properties.at(tag).at("death"), offset));
			// Record the time when vaccine effects start dropping (assumes all these properties follow the same trend)
			agent.set_time_vaccine_effects_reduction(offset+vac_types_properties.at(tag).at("effectiveness").at(3).at(0));
			// and the time when mobility increases (at peak effectiveness)
			agent.set_time_mobility_increase(offset+vac_types_properties.at(tag).at("effectiveness").at(2).at(0));
		}
	}
}
