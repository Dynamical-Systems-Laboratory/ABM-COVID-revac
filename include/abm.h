#ifndef ABM_H
#define ABM_H

#include "abm_include.h"

/***************************************************** 
 * class: ABM
 * 
 * Interface for agent-based modeling 
 *
 * Provides operations for creation, management, and
 * progression of an agent-based model
 *
 * Stores model-related data - for output options
 * check the DataManagementInterface class
 *
 * NOTE: IDs of objects correspond to their positions
 * in the vectors of objects and determine the way
 * they are accessed; IDs start with 1 but are corrected
 * by -1 when accessing; i.e. object with ID = 3, is 
 * stored at index 2 of the corresponding vector  
 * 
 ******************************************************/

class DataManagementInterface;

class ABM : public DataManagementInterface {
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates an ABM object with default attributes
	 */
	ABM() : DataManagementInterface() { }

	/**
	 * \brief Creates an ABM object with custom attributes
	 *
	 * @param del_t - time step, days
	 * @param infile - name of the file with the input parameters
	 * @param dist_files - map of keys-tags and file names where different distribution files are stored 
	 * @param tfile - file with time dependent testing parameters
	 * @param vfile - file with vaccination parameters
 	 * @param vdata - path to the directory with vaccination type tables 
	 */
	ABM(double del_t, const std::string infile, const std::map<std::string, std::string> dist_files,
					const std::string tfile, const std::string vfile, const std::string vdata) : 
					dt(del_t), infection(del_t), vaccinations(vfile, vdata), DataManagementInterface() 
	{
		time = 0.0;
		load_infection_parameters(infile); 
		load_age_dependent_distributions(dist_files);
		load_testing(tfile);
		initialize_data_collection();
	}
	
	/**
	 * \brief Creates an ABM object assuming the simulation setup happens separately in full
	 * \details This does not load any parameters or setup objects except Infection and
					DataMangement. It initializes part of the data members, 
	 *				but the user needs to use this constructor with the setup_simulation function 
	 *
	 * @param del_t - time step, days
	 */
	ABM(double del_t) :	dt(del_t), infection(del_t), DataManagementInterface() 
	{
		time = 0.0;
		initialize_data_collection();
	}

	//
	// Initialization and object construction
	//

	/**
	 * \brief Create the town, agents, and introduce initially infected 
	 * \details Performs all basic setup operations; the file with 
	 * input information has filenames of all input files, tagged
	 * See examples of usage in testing and simulation directories. 
	 * This sets up the simulation core, custom extensions - like
	 * vaccinating and intializing active cases need to be done 
	 * separately, by the user.
	 *	
	 * @param filename - path of the file with input information
	 * @param ninf0 - number of initially infected - overwriting input file
	 *
	 */	
	void simulation_setup(const std::string filename, const int ninf0 = 0);

	/**
	 * \brief Create households based on information in a file
	 * \details Constructs households based on the ID and
	 * 				locations as defined in the file; One line
	 * 				in the file defines one household 
	 *	
	 * @param filename - path of the file with input information
	 * 
	 */	
	void create_households(const std::string filename);

	/**
	 * \brief Create retirement homes based on information in a file
	 * \details Constructs retirement homes based on the ID and
	 * 				locations as defined in the file; One line
	 * 				in the file defines one household 
	 *	
	 * @param filename - path of the file with input information
	 * 
	 */	
	void create_retirement_homes(const std::string filename);

	/**
	 * \brief Create schools based on information in a file
	 * \details Constructs schools based on the ID,
	 * 				locations, and school type as defined in the file; 
	 * 				One line in the file defines one school;
	 * 				School types are "daycare", "primary", "middle",
	 * 				"high", and "college"  
	 *	
	 * @param filename - path of the file with input information
	 * 
	 */	
	void create_schools(const std::string filename);

	/**
	 * \brief Create workplaces based on information in a file
	 * \details Constructs workplaces based on the ID and
	 * 				locations as defined in the file; One line
	 * 				in the file defines one workplace 
	 *	
	 * @param filename - path of the file with input information
	 * 
	 */	
	void create_workplaces(const std::string filename);

	/**
	 * \brief Create hospitals based on information in a file
	 * \details Constructs hospitals based on the ID and
	 * 				locations as defined in the file; One line
	 * 				in the file defines one hospital 
	 *	
	 * @param filename - path of the file with input information
	 * 
	 */	
	void create_hospitals(const std::string filename);

	/**
	 * \brief Create carpool objects based on information in a file
	 * \details Constructs carpools based on the ID and
	 * 				locations as defined in the file; One line
	 * 				in the file defines one carpool 
	 *	
	 * @param filename - path of the file with input information
	 * 
	 */	
	void create_carpools(const std::string filename);

	/**
	 * \brief Create public transit objects based on information in a file
	 * \details Constructs public transit based on the ID and
	 * 				locations as defined in the file; One line
	 * 				in the file defines one public transit 
	 *	
	 * @param filename - path of the file with input information
	 * 
	 */	
	void create_public_transit(const std::string filename);

	/**
	 * \brief Create weekdend/leisure objects based on information in a file
	 * \details Constructs leisure locations based on the ID and
	 * 				locations as defined in the file; One line
	 * 				in the file defines one leisure location
	 *	
	 * @param filename - path of the file with input information
	 * 
	 */	
	void create_leisure_locations(const std::string filename);

	/**
	 * \brief Initialize Mobility and assignment of leisure locations
	 */
	void initialize_mobility();

	/**
	 * \brief Create agents based on information in a file
	 * \details Constructs agents from demographic information
	 * 		in a file with agent per row, columns being the
	 *		information as it currently appears in the Agent
	 *		constructor; assigns agents to households, schools,
	 *		workplaces, and hospitals - needs to be called AFTER creating
	 *		those places; Optionally it can randomly choose initially
	 *		infected as specified by the input parameter;
	 *	
	 * @param filename - path of the file with input information
	 * @param ninf0 - number of initially infected - overwriting input file
	 * 
	 */	
	void create_agents(const std::string filename, const int ninf0 = 0);

	/// Start with N_inf agents that have COVID-19 in various stages
	/// @param vaccinate - false means this will not initialize various vaccinated stages
	/// @param N_vac - number of agents to vaccinate as part of seeding (if any) 
	void initialize_active_cases(const int N_inf, const bool vaccinate = false, const int N_vac = 0);

	/// Set up vaccination of nv members of the random population members activated with testing
	void set_random_vaccination(int nv) 
		{ random_vaccines = true; n_vaccinated = nv;}
	/// Set up vaccination of specific population group activated with testing
	/// @param vaccine_group_name - recognizable name of the group (see manual)
	/// @param verbose - print the final number of vaccinated individuals
	void set_group_vaccination(std::string group_name, bool verbose = false) 
		{ group_vaccines = true; vaccine_group_name = group_name; vac_verbose = verbose; } 
	
	/// Initialization for vaccination vs. reopening studies
	/// @param dont_vac - dont vaccinate at this stage (e.g vaccinate in the seeding phase)
	void initialize_vac_and_reopening(const bool dont_vac = false);

	//
	// Transmission of infection
	//

	/// Transmit infection - original way
	/// @details Time-dependent testing, closures, reopenings 
	void transmit_infection();

	/// Transmit with constant testing and vaccination rate 
	void transmit_with_vac();

   /**
	* \brief Perfect testing, vaccinations, and reopening
	* \details Run the simulation with fixed testing 
	* 	and no closures/reopenings - instead, constant
	*  	reopening modification or time rates; and
	* 	daily increase in vaccinated population 
	*/
	void transmit_ideal_testing_vac_reopening();

	/// Assign leisure locations for this step
	void distribute_leisure();  

	/// Update transmission dynamics in wokplaces outside of the town
	void set_outside_workplace_transmission();
	
	/// Update transmission dynamics in leisure locations outside of the town
	void set_outside_leisure_transmission();

	/// \brief Count contributions of all infectious agents in each place 
	void compute_place_contributions();

	/// \brief Propagate infection and determine state transitions
	void compute_state_transitions();

	/// \brief Set the lambda factors to 0.0
	void reset_contributions()
		{ contributions.reset_sums(households, schools, workplaces, hospitals, 
						retirement_homes, carpools, public_transit, leisure_locations); }
	
	/// Process all traced agents 
	void setup_traced_isolation(const std::unordered_set<int>&);

	// Increasing time
	void advance_in_time() { time += dt; }

	/// Verify if anything that requires parameter changes happens at this step 
	void check_events(std::vector<School>&, std::vector<Workplace>&);

	//
	// Getters
	//
	
	/// Current simulation time
	double get_time() const { return time; }

	/// Saves the matrix with mobility probabilities
	void print_mobility_probabilities(const std::string fname)
		{ mobility.print_probabilities(fname); }

	/// Calculate the average number of contacts an agent makes
	/// \detail Does not count contacts due to treatment or testing
	double get_average_contacts();

	/**
	 * \brief Save infection parameter information
	 *
     * @param filename - path of the file to print to
	 */
	void print_infection_parameters(const std::string filename) const;	

	/**
	 * \brief Save age-dependent distributions 
	 *
     * @param filename - path of the file to print to
	 */
	void print_age_dependent_distributions(const std::string filename) const;

	/// Return a copy of Infection object
	Infection get_copied_infection_object() const { return infection; }
	/// Return a reference to an Infection object
	Infection& get_infection_object() { return infection; }
	/// Return a const reference to parameter map
	const std::map<std::string, double>& get_infection_parameters() const
		{ return infection_parameters; }
	/// Return a reference to parameter map
	std::map<std::string, double>& get_infection_parameters() 
		{ return infection_parameters; }
	/// Return a copy of the Flu object
	Flu get_flu_object() const { return flu; }
	/// Return a reference to Flu object
	Flu& get_flu_object() { return flu; }
	/// Return a copy of the Testing object
	Testing get_testing_object() const { return testing; }
	/// Return the Testing object
	Testing& get_testing_object() { return testing; }
	/// Return a Transitions object
	Transitions& get_transitions_object() { return transitions; }
private:

	// General model attributes
	// Time step
	double dt = 1.0;
	// Time - updated contiuously throughout the simulation
	double time = 0.0;
	
	// Infection parameters
	std::map<std::string, double> infection_parameters = {};
	// Age-dependent distributions
	std::map<std::string, std::map<std::string, double>> age_dependent_distributions = {};

	// Infection properties and transmission model
	Infection infection;
	// Performs vaccinations and manages properties of vaccines
	Vaccinations vaccinations;
	// Testing properties and their time dependence
	Testing testing;	
	// Class for selecting contact traced agents
	Contact_tracing contact_tracing;
	// Part of the mobility functionality
	Mobility mobility;
	// Class for computing infection contributions
	Contributions contributions;
	// Class for computing agent transitions
	Transitions transitions;
	// Class for setting agent state transitions
	StatesManager states_manager;
	// Class for creating and maintaining a population
	// with flu i.e.  non-covid symptomatic
	Flu flu;

	// Vaccination properties
	bool random_vaccines = false;
	int n_vaccinated = 0;
	bool group_vaccines = false;
	std::string vaccine_group_name;
	bool vac_verbose = false;
	
	// Leisure properties
	// Initial transmissiont rate
	double ini_beta_les = 0.0;
	// Difference between initial and final transmission rate
	double del_beta_les = 0.0;
	// Initial fraction going to leisure locations
	double ini_frac_les = 0.0;
	// Difference between initial and final fraction
	double del_frac_les = 0.0;

	// Private methods

	/// Set initial values on all the data collection variables and containers
	void initialize_data_collection();

	/// Load infection parameters, store in a map
	void load_infection_parameters(const std::string);

	/// Load age-dependent distributions as vectors stored in a map
	void load_age_dependent_distributions(const std::map<std::string, std::string>);

	/// Initialize testing and its time dependence
	void load_testing(const std::string);
	
	/// Initialize Vaccinations class 
	void load_vaccinations(const std::string, const std::string);
	
	/// \brief Set properties of initially infected - exposed
	void initial_exposed(Agent&);

	/// Set up contact tracing functionality
	void initialize_contact_tracing();

	/// Initialize an asymptomatic agent, randomly in the course of disease
	void process_initial_asymptomatic(Agent& agent);
	/// Initialize a symptomatic agent, randomly in the course of disease
	void process_initial_symptomatic(Agent& agent);

	/// Vaccinate random members of the population that are not Flu or infected agents
	void vaccinate_random();
	/// Vaccinate specific group of agents in the population
	void vaccinate_group();
	/// Vaccinate random members of the population with a variable time offset 
	/// This will subtract a time between t0 and tf from the agent vaccination functions
	/// Making it work as if they were vaccinated earlier  
	void vaccinate_random_time_offset();
	///	Randomly vaccinate agents based on the daily rate
	void vaccinate();  
	/// Increase transmission rate and visiting frequency of leisure locations 
	void reopen_leisure_locations();

	/// Checks if agent is in a condition that allows going to leisure locations
	bool check_leisure_eligible(const Agent& agent, const int);

	/// Finds the actual leisure location and registers eligible agent(s)
	void check_select_and_register_leisure_location(const std::vector<int>& agent_IDs, const int house_ID);

	/// Initiate contact tracing of an agent
	void contact_trace_agent(Agent& agent);
	/**
	 * \brief Retrieve information about agents from a file and store all in a vector
	 * \details Optional parameter overwrites the loaded initially infected with custom
	 */
	void load_agents(const std::string fname, const int ninf0 = 0);

	/**
	 * \brief Assign agents to households, schools, and worplaces
	 */
	void register_agents();

	/// Start detection, initialize agents with flu, vaccinate
	/// @param dont_vac - dont vaccinate at this stage (e.g vaccinate in the seeding phase)
	void start_testing_flu_and_vaccination(const bool dont_vac = false);
};

#endif
