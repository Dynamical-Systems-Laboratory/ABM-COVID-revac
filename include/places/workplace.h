#ifndef WORKPLACE_H
#define WORKPLACE_H

#include "place.h"

class Place;

/***************************************************** 
 * class: Workplace 
 * 
 * Defines and stores attributes of a single workplace
 * 
 *****************************************************/

class Workplace : public Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Workplace object with default attributes
	 */
	Workplace() = default;

	/**
	 * \brief Creates a Workplace object 
	 * \details Workplace with custom ID, location, and infection parameters
	 *
	 * @param work_ID - ID of the workplace
	 * @param xi - x coordinate of the workplace 
	 * @param yi - y coordinate of the workplace
	 * @param severity_cor - severity correction for symptomatic
	 * @param psi - absenteeism correction
	 * @param beta - infection transmission rate, 1/time
	 * @param wtype - workplace type
	 */
	Workplace(const int work_ID, const double xi, const double yi,
			 const double severity_cor, const double psi, const double beta, 
			 const std::string wtype) : 
			psi_j(psi), type(wtype), Place(work_ID, xi, yi, severity_cor, beta){ }

	//
	// Infection related computations
	//

	/** 
	 *  \brief Include symptomatic contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 */
	void add_symptomatic(double inf_var) override { lambda_sum += inf_var*ck*beta_j*psi_j; }

	
	/// Calculates and stores probability contribution of infected agents if any 
	void compute_infected_contribution() override;

	//
	// Setters
	//
	
	void change_absenteeism_correction(const double val) { psi_j = val; }
	
	/// Assign total contribution at a workplace outside the modeled town
	void set_outside_lambda(const double val) { lam_tot_out = val; }

	/// Separate modification due to closures
	void adjust_outside_lambda(const double mod) { lam_tot_out *= mod; }

	//
	// Getters
	//
	
	double get_absenteeism_correction() const { return psi_j; }
	
	/// True if the workplace is outside current town
	bool outside_town() const override 
		{ return (type == "outside")?(true) : (false); }

	/// Lambda for a workplace outside the modeled town
	double get_outside_lambda() const { return lam_tot_out; }

	/// Occupation type of this workplace
	std::string get_type() const override { return type; } 

	//
 	// I/O
	//

	/**
	 * \brief Save information about a Workplace object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		number of infected agents | ck | beta_j | psi_j 
	 * 		Delimiter is a space.
	 * 	@param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Absenteeism correction
	double psi_j = 0.0;
	// Workplace type 
	std::string type = "none";
	// Lambda of an outside workplace
	double lam_tot_out = 0.0;
};
#endif
