/*
 * LtlParser.cpp
 *
 *  Created on: 22.04.2013
 *      Author: thomas
 */

#include "LtlParser.h"

#include "src/utility/OsDetection.h"
#include "src/utility/ConstTemplates.h"

// If the parser fails due to ill-formed data, this exception is thrown.
#include "src/exceptions/WrongFormatException.h"

// Used for Boost spirit.
#include <boost/typeof/typeof.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/phoenix.hpp>

// Include headers for spirit iterators. Needed for diagnostics and input stream iteration.
#include <boost/spirit/include/classic_position_iterator.hpp>
#include <boost/spirit/include/support_multi_pass.hpp>

// Needed for file IO.
#include <fstream>
#include <iomanip>
#include <map>


// Some typedefs and namespace definitions to reduce code size.
typedef std::string::const_iterator BaseIteratorType;
typedef boost::spirit::classic::position_iterator2<BaseIteratorType> PositionIteratorType;
namespace qi = boost::spirit::qi;
namespace phoenix = boost::phoenix;



namespace storm {

namespace parser {

template<typename Iterator, typename Skipper>
struct LtlParser::LtlGrammar : qi::grammar<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper > {
	LtlGrammar() : LtlGrammar::base_type(start) {
		freeIdentifierName = qi::lexeme[+(qi::alpha | qi::char_('_'))];

		//This block defines rules for parsing state formulas
		ltlFormula %= orFormula;
		ltlFormula.name("state formula");
		orFormula = andFormula[qi::_val = qi::_1] > *(qi::lit("|") > andFormula)[qi::_val =
				phoenix::new_<storm::formula::ltl::Or<double>>(qi::_val, qi::_1)];
		orFormula.name("state formula");
		andFormula = notFormula[qi::_val = qi::_1] > *(qi::lit("&") > notFormula)[qi::_val =
				phoenix::new_<storm::formula::ltl::And<double>>(qi::_val, qi::_1)];
		andFormula.name("state formula");
		notFormula = atomicLtlFormula[qi::_val = qi::_1] | (qi::lit("!") > atomicLtlFormula)[qi::_val =
				phoenix::new_<storm::formula::ltl::Not<double>>(qi::_1)];
		notFormula.name("state formula");

		//This block defines rules for "atomic" state formulas
		//(Propositions, probabilistic/reward formulas, and state formulas in brackets)
		atomicLtlFormula %= pathFormula | atomicProposition | qi::lit("(") >> ltlFormula >> qi::lit(")");
		atomicLtlFormula.name("state formula");
		atomicProposition = (freeIdentifierName)[qi::_val =
				phoenix::new_<storm::formula::ltl::Ap<double>>(qi::_1)];
		atomicProposition.name("state formula");
		/*probabilisticBoundOperator = (
				(qi::lit("P") >> qi::lit(">") >> qi::double_ > qi::lit("[") > LtlFormula > qi::lit("]"))[qi::_val =
						phoenix::new_<storm::formula::ltl::ProbabilisticBoundOperator<double> >(storm::formula::GREATER, qi::_1, qi::_2)] |
				(qi::lit("P") >> qi::lit(">=") > qi::double_ > qi::lit("[") > LtlFormula > qi::lit("]"))[qi::_val =
						phoenix::new_<storm::formula::ltl::ProbabilisticBoundOperator<double> >(storm::formula::GREATER_EQUAL, qi::_1, qi::_2)] |
				(qi::lit("P") >> qi::lit("<") >> qi::double_ > qi::lit("[") > LtlFormula > qi::lit("]"))[qi::_val =
								phoenix::new_<storm::formula::ltl::ProbabilisticBoundOperator<double> >(storm::formula::LESS, qi::_1, qi::_2)] |
				(qi::lit("P") >> qi::lit("<=") > qi::double_ > qi::lit("[") > LtlFormula > qi::lit("]"))[qi::_val =
						phoenix::new_<storm::formula::ltl::ProbabilisticBoundOperator<double> >(storm::formula::LESS_EQUAL, qi::_1, qi::_2)]
				);
		probabilisticBoundOperator.name("state formula");*/

		//This block defines rules for parsing formulas with noBoundOperators
		/*noBoundOperator = (probabilisticNoBoundOperator | rewardNoBoundOperator);
		noBoundOperator.name("no bound operator");
		probabilisticNoBoundOperator = (qi::lit("P") >> qi::lit("=") >> qi::lit("?") >> qi::lit("[") >> LtlFormula >> qi::lit("]"))[qi::_val =
				phoenix::new_<storm::formula::ltl::ProbabilisticNoBoundOperator<double> >(qi::_1)];
		probabilisticNoBoundOperator.name("no bound operator");*/

		//This block defines rules for parsing probabilistic path formulas
		pathFormula = (boundedEventually | eventually | globally | boundedUntil | until);
		pathFormula.name("ltl formula");
		boundedEventually = (qi::lit("F") >> qi::lit("<=") > qi::int_ > ltlFormula)[qi::_val =
				phoenix::new_<storm::formula::ltl::BoundedEventually<double>>(qi::_2, qi::_1)];
		boundedEventually.name("ltl formula");
		eventually = (qi::lit("F") > ltlFormula)[qi::_val =
				phoenix::new_<storm::formula::ltl::Eventually<double> >(qi::_1)];
		eventually.name("ltl formula");
		globally = (qi::lit("G") > ltlFormula)[qi::_val =
				phoenix::new_<storm::formula::ltl::Globally<double> >(qi::_1)];
		globally.name("ltl formula");
		boundedUntil = (ltlFormula[qi::_a = phoenix::construct<std::shared_ptr<storm::formula::ltl::AbstractLtlFormula<double>>>(qi::_1)] >> qi::lit("U") >> qi::lit("<=") > qi::int_ > ltlFormula)
				[qi::_val = phoenix::new_<storm::formula::ltl::BoundedUntil<double>>(phoenix::bind(&storm::formula::ltl::AbstractLtlFormula<double>::clone, phoenix::bind(&std::shared_ptr<storm::formula::ltl::AbstractLtlFormula<double>>::get, qi::_a)), qi::_3, qi::_2)];
		boundedUntil.name("ltl formula");
		until = (ltlFormula[qi::_a = phoenix::construct<std::shared_ptr<storm::formula::ltl::AbstractLtlFormula<double>>>(qi::_1)] >> qi::lit("U") > ltlFormula)[qi::_val =
				phoenix::new_<storm::formula::ltl::Until<double>>(phoenix::bind(&storm::formula::ltl::AbstractLtlFormula<double>::clone, phoenix::bind(&std::shared_ptr<storm::formula::ltl::AbstractLtlFormula<double>>::get, qi::_a)), qi::_2)];
		until.name("ltl formula");

		start = ltlFormula;
		start.name("LTL formula");
	}

	qi::rule<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper> start;

	qi::rule<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper> ltlFormula;
	qi::rule<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper> atomicLtlFormula;

	qi::rule<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper> andFormula;
	qi::rule<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper> atomicProposition;
	qi::rule<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper> orFormula;
	qi::rule<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper> notFormula;
	//qi::rule<Iterator, storm::formula::ltl::ProbabilisticBoundOperator<double>*(), Skipper> probabilisticBoundOperator;

	//qi::rule<Iterator, storm::formula::ltl::AbstractNoBoundOperator<double>*(), Skipper> noBoundOperator;
	//qi::rule<Iterator, storm::formula::ltl::AbstractNoBoundOperator<double>*(), Skipper> probabilisticNoBoundOperator;

	qi::rule<Iterator, storm::formula::ltl::AbstractLtlFormula<double>*(), Skipper> pathFormula;
	qi::rule<Iterator, storm::formula::ltl::BoundedEventually<double>*(), Skipper> boundedEventually;
	qi::rule<Iterator, storm::formula::ltl::Eventually<double>*(), Skipper> eventually;
	qi::rule<Iterator, storm::formula::ltl::Globally<double>*(), Skipper> globally;
	qi::rule<Iterator, storm::formula::ltl::BoundedUntil<double>*(), qi::locals< std::shared_ptr<storm::formula::ltl::AbstractLtlFormula<double>>>, Skipper> boundedUntil;
	qi::rule<Iterator, storm::formula::ltl::Until<double>*(), qi::locals< std::shared_ptr<storm::formula::ltl::AbstractLtlFormula<double>>>, Skipper> until;

	qi::rule<Iterator, std::string(), Skipper> freeIdentifierName;

};

} //namespace storm
} //namespace parser


storm::parser::LtlParser::LtlParser(std::string formulaString) {
	// Prepare iterators to input.
	BaseIteratorType stringIteratorBegin = formulaString.begin();
	BaseIteratorType stringIteratorEnd = formulaString.end();
	PositionIteratorType positionIteratorBegin(stringIteratorBegin, stringIteratorEnd, formulaString);
	PositionIteratorType positionIteratorEnd;


	// Prepare resulting intermediate representation of input.
	storm::formula::ltl::AbstractLtlFormula<double>* result_pointer = nullptr;

	LtlGrammar<PositionIteratorType,  BOOST_TYPEOF(boost::spirit::ascii::space)> grammar;

	// Now, parse the formula from the given string
	try {
		qi::phrase_parse(positionIteratorBegin, positionIteratorEnd, grammar, boost::spirit::ascii::space, result_pointer);
	} catch(const qi::expectation_failure<PositionIteratorType>& e) {
		// If the parser expected content different than the one provided, display information
		// about the location of the error.
		const boost::spirit::classic::file_position_base<std::string>& pos = e.first.get_position();

		// Construct the error message including a caret display of the position in the
		// erroneous line.
		std::stringstream msg;
		msg << pos.file << ", line " << pos.line << ", column " << pos.column
				<< ": parse error: expected " << e.what_ << std::endl << "\t"
				<< e.first.get_currentline() << std::endl << "\t";
		int i = 0;
		for (i = 0; i < pos.column; ++i) {
			msg << "-";
		}
		msg << "^";
		for (; i < 80; ++i) {
			msg << "-";
		}
		msg << std::endl;

		std::cerr << msg.str();

		// Now propagate exception.
		throw storm::exceptions::WrongFormatException() << msg.str();
	}

	// The syntax can be so wrong that no rule can be matched at all
	// In that case, no expectation failure is thrown, but the parser just returns nullptr
	// Then, of course the result is not usable, hence we throw a WrongFormatException, too.
	if (result_pointer == nullptr) {
		throw storm::exceptions::WrongFormatException() << "Syntax error in formula";
	}

	formula = result_pointer;
}

