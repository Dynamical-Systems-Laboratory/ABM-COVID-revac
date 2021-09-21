#ifndef TRANSIT_H
#define TRANSIT_H

#include "place.h"

class Place;

/***************************************************** 
 * class: Transit 
 * 
 * Defines and stores attributes of a single transit 
 * mode object 
 * 
 *****************************************************/

class Transit : public Place{
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Transit object with default attributes
	 */
	Transit() = default;

	/**
	 * \brief Creates a Transit object 
	 * \details Transit with custom ID, type, and infection parameters
	 *
	 * @param transit_ID - ID of the transit object 
	 * @param severity_cor - severity correction for symptomatic
	 * @param psi - absenteeism correction
	 * @param beta - infection transmission rate, 1/time
	 * @param tr_type - transit type
	 */
	Transit(const int transit_ID, const double beta,
				const double severity_cor, const double psi, 
				const std::string tr_type) : 
		psi_j(psi), type(tr_type), Place(transit_ID, 0.0, 0.0, severity_cor, beta){ }

	//
	// Infection related computations
	//

	/** 
	 *  \brief Include symptomatic contribution in the sum
	 *	@param inf_var - agent infectiousness variability factor
	 */
	void add_symptomatic(double inf_var) override { lambda_sum += inf_var*ck*beta_j*psi_j; }

	/** 
	 *  \brief Include symptomatic contribution in the sum with non-default absenteeism correction
	 *	@param inf_var - agent infectiousness variability factor
	 *  @param psi - absenteeism correction for that agent's category
	 */
	void add_special_symptomatic(const double inf_var, const double psi) { lambda_sum += inf_var*ck*beta_j*psi; }

	//
	// Setters
	//
	
	void change_absenteeism_correction(const double val) { psi_j = val; }
	
	//
	// Getters
	//
	
	double get_absenteeism_correction() const { return psi_j; }

	//
 	// I/O
	//

	/**
	 * \brief Save information about a Transit object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		number of infected agents | ck | beta_j | psi_j | type 
	 * 		Delimiter is a space.
	 * 	@param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Absenteeism correction
	double psi_j = 0.0;
	// Transit type 
	std::string type;
};
#endif
