#ifndef LEISURE_H
#define LEISURE_H

#include "place.h"

class Place;

/***************************************************** 
 * class: Leisure 
 * 
 * Defines and stores attributes of a single leisure 
 * location 
 * 
 *****************************************************/

class Leisure : public Place {
public:

	//
	// Constructors
	//

	/**
	 * \brief Creates a Leisure object with default attributes
	 */
	Leisure() = default;

	/**
	 * \brief Creates a Leisure object 
	 * \details Leisure with custom ID, location, and infection parameters
	 *
	 * @param leisure_ID - ID of the leisure location 
	 * @param xi - x coordinate of the leisure location 
	 * @param yi - y coordinate of the leisure location
	 * @param severity_cor - severity correction for symptomatic
	 * @param beta - infection transmission rate, 1/time
	 * @param ltype - leisure location type
	 */
	Leisure(const int leisure_ID, const double xi, const double yi,
			 const double severity_cor, const double beta,  
			 const std::string ltype) : 
			type(ltype), Place(leisure_ID, xi, yi, severity_cor, beta){ }

	//
	// Infection related computations
	//

	/// Calculates and stores probability contribution of infected agents if any 
	void compute_infected_contribution() override;
	
	//
	// Setters
	//

	/// Assign total contribution at a leisure location outside the modeled town
	void set_outside_lambda(const double val) { lam_tot_out = val; }

	/// Separate modification due to closures
	void adjust_outside_lambda(const double mod) { lam_tot_out *= mod; }
	
	//
	// Getters
	//
	
	/// True if the leisure location is outside current town
	bool outside_town() const override 
		{ return (type == "outside")?(true) : (false); }

	/// Lambda for a leisure location outside the modeled town
	double get_outside_lambda() const { return lam_tot_out; }
	
	//
 	// I/O
	//

	/**
	 * \brief Save information about a Leisure object
	 * \details Saves to a file, everything but detailed agent 
	 * 		information; order is ID | x | y | number of agents | 
	 * 		number of infected agents | ck | beta_j | type 
	 * 		Delimiter is a space.
	 * 	@param where - output stream
	 */
	void print_basic(std::ostream& where) const override;

private:
	// Leisure location type 
	std::string type = "none";
	// Lambda of an outside leisure location
	double lam_tot_out = 0.0;
};
#endif
