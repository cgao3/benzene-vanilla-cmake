//----------------------------------------------------------------------------
/** @file PatternPrecomputation.cpp */
//----------------------------------------------------------------------------

#include "Pattern.hpp"
#include "Misc.hpp"

using namespace benzene ;

//----------------------------------------------------------------------------

/**

    This executable generates ice-patterns.txt from fillin-patterns.txt and
  misc-patterns.txt.
    It assumes fillin-patterns.txt to contain only EITHER_FILLIN and
  WHITE_FILLIN patterns. From them, it derives all the inferior and strong
  reversible patterns it can.
    Actually, there can also be WHITE_STRONG_REVERSIBLE or WHITE_INFERIOR ones
  that would virtually be obtained by deriving, but are not for specific
  optimising tricks.
    The patterns from misc-patterns.txt can be anything, and are essentially
  only copied.

    By default, it derives strong reversible and inferior patterns from the
  fillin patterns in fillin-patterns.txt. This can be changed by the argument
  "no-s_r" or "no-inf".
    Some inferior patterns in misc-patterns.txt are obtained by iterative
  fillin and have comment "it", they can be ignored by the argument
  "no-it". The "no-inf" argument also has this secondary effect.
    Some patterns are "big", by default they are unused. They can be used by
  the argument "use-big".

    The patterns with "no-s_r" (resp. "no-inf") generate no strong reversible
  (resp. no inferior) pattern.
    The patterns with "deduce-only" will not be included, but will only be
  used to deduce strong inferior and deduce patterns.

*/

//----------------------------------------------------------------------------

namespace {

const char* build_date = __DATE__;

}

//----------------------------------------------------------------------------

struct PatterPrecomputationInfos
{
public:
    int e_fillin_copied = 0;
    int e_fillin_ignored = 0;
    int fillin_copied = 0;
    int fillin_ignored = 0;
    int s_reversible_derived = 0;
    int s_reversible_copied = 0;
    int inferior_derived = 0;
    int inferior_copied = 0;
    int t_reversible_copied = 0;
    int reversible_copied = 0;
    int big_ignored = 0;
    int it_ignored = 0;
};

inline
bool Contains(const Pattern pat, std::string str)
{
    return pat.GetComment().find(str) != std::string::npos ;
}
  
  
/** Adds all specific variations where one element of feat_color has been
    replaced by an element of FEATURE_MARKED2 to give a pattern of type */
void AddSpecVariations(Pattern pat, int feat_color,
		       char type, std::ofstream& out,
		       int& ppi_derived)
{
    const Pattern::slice_t* data = pat.GetData();
    int pattern_number = 0;
    for (int s=0; s<Pattern::NUM_SLICES; s++)
    {
        int stones = data[s][feat_color] ;
        for (int pow=1; stones>0; pow = 2*pow)
	{
	    if (stones%(2*pow)==pow) // ie pow appears in stones
	    {
	        stones = stones-pow;

	        ++ppi_derived;
		// Note : not always injective
		out << " [" << type << pattern_number++ << "_from_"
		    << pat.GetName() << "/]" << std::endl ;

		out << type << ":" ;
		for (int s2=0; s2<Pattern::NUM_SLICES; s2++)
		{
		    for (int f=0; f<Pattern::NUM_FEATURES; f++)
		    {
			if (s2 == s & f == feat_color)
			    out << (data[s][feat_color] - pow) ;
			else if (s2 == s & f == Pattern::FEATURE_MARKED2)
			    out << pow ;
			else if (f == Pattern::FEATURE_MARKED2)
			    out << "0" ;
			else
			    out << data[s2][f] ;
			if (f < Pattern::NUM_FEATURES -1) {out << "," ;}
			else {out << ";" ;}
		    }
		}
		out << std::endl ;
	    }
        }
    }
}

/** Adds all the variations of a fillin pattern */
void AddVariations(Pattern pat, std::ofstream& out,
		   bool deduce_s_reversible, bool deduce_inferior,
		   bool use_big, PatterPrecomputationInfos& ppi)
{
    // If a pattern is "big", by default it is ignored.
    if (!use_big && Contains(pat,"big"))
    {
        ++ppi.big_ignored;
        return;
    }

    /*if (Contains(pat,"nd"))
    {
        return;
    }

    if (Contains(pat,"npi"))
    {
        return;
	}*/

    char type = pat.GetType() ;
    
    // If a pattern is "deduce-only", do not include.
    if (!Contains(pat,"deduce-only"))
    {
        switch (type)
	{
	case Pattern::WHITE_FILLIN :
	    ++ppi.fillin_copied;
	    break;
	case Pattern::EITHER_FILLIN :
	    ++ppi.e_fillin_copied;
	    break;
	case Pattern::WHITE_STRONG_REVERSIBLE :
	    ++ppi.s_reversible_derived;
	    break;
	case Pattern::WHITE_INFERIOR :
	    ++ppi.inferior_derived;
	    break;
	default :
	    throw BenzeneException() << "Bad type in fillin-patterns.txt: "
				     << type ;
	}
	out << " [" << pat.GetName() << "/]" << std::endl ;
	out << pat.Serialize() << std::endl ;
    }
    else
    {
        switch (type)
	{
	case Pattern::WHITE_FILLIN :
	    ++ppi.fillin_ignored;
	    break;
	case Pattern::EITHER_FILLIN :
	    ++ppi.e_fillin_ignored;
	    break;
	default :
	    throw BenzeneException() << "Bad type in fillin-patterns.txt: "
				     << type ;
	}
    }
    
    switch (type)
    {
    case Pattern::WHITE_FILLIN :
        if (deduce_s_reversible && !Contains(pat,"no-s_r"))
	    AddSpecVariations(pat, Pattern::FEATURE_BLACK,
			      Pattern::WHITE_STRONG_REVERSIBLE, out,
			      ppi.s_reversible_derived);
	if (deduce_inferior && !Contains(pat,"no-inf"))
	    AddSpecVariations(pat, Pattern::FEATURE_WHITE,
			      Pattern::WHITE_INFERIOR, out,
			      ppi.inferior_derived);
	break;
    case Pattern::EITHER_FILLIN :
        if (deduce_s_reversible && !Contains(pat,"no-s_r"))
	    AddSpecVariations(pat, Pattern::FEATURE_BLACK,
			      Pattern::WHITE_STRONG_REVERSIBLE, out,
			      ppi.s_reversible_derived);
	if (deduce_inferior && !Contains(pat,"no-inf"))
	    AddSpecVariations(pat, Pattern::FEATURE_WHITE,
			      Pattern::WHITE_INFERIOR, out,
			      ppi.inferior_derived);
	pat.FlipColors();
        if (deduce_s_reversible && !Contains(pat,"no-s_r"))
	    AddSpecVariations(pat, Pattern::FEATURE_BLACK,
			      Pattern::WHITE_STRONG_REVERSIBLE, out,
			      ppi.s_reversible_derived);
	if (deduce_inferior && !Contains(pat,"no-inf"))
	    AddSpecVariations(pat, Pattern::FEATURE_WHITE,
			      Pattern::WHITE_INFERIOR, out,
			      ppi.inferior_derived);
	break;
    case Pattern::WHITE_STRONG_REVERSIBLE :
    case Pattern::WHITE_INFERIOR :
	break;
    default :
	throw BenzeneException() << "Bad type in fillin-patterns.txt: "
				 << type ;
    }
}

int main( int argc, char *argv[] )
{
    bool deduce_inferior = true ;
    bool deduce_s_reversible = true ;
    bool iterative_inferior = true ;
    bool use_big = false ;
    for (int i=1; i<argc; ++i)
    {
	std::string arg = argv[i];
        if (arg == "no-inf")
	{
	    deduce_inferior = false;
	    iterative_inferior = false;
	}
	else if (arg == "no-s_r")
	    deduce_s_reversible = false;
	else if (arg == "no-it")
	    iterative_inferior = false;
	else if (arg == "use-big")
	    use_big = true;
	else {throw BenzeneException()
	    << "Arguments: no-inf, no-s_r, no-it or use-big.";};
    }
    
    std::ifstream inFile;
    try {
        MiscUtil::OpenFile("fillin-patterns.txt", inFile);
    }
    catch (BenzeneException& e) {
        throw BenzeneException() << "PatternPrecomputation: " << e.what();
    }
    std::vector<Pattern> fillin;
    if (inFile.is_open())
    {
	Pattern::LoadPatternsFromStream(inFile, fillin);
	inFile.close();
    }
    else {throw BenzeneException() << "Could not open fillin-patterns.txt";};
    
    try {
        MiscUtil::OpenFile("misc-patterns.txt", inFile);
    }
    catch (BenzeneException& e) {
        throw BenzeneException() << "PatternPrecomputation: " << e.what();
    }
    std::vector<Pattern> misc;
    if (inFile.is_open())
    {
        Pattern::LoadPatternsFromStream(inFile, misc);
	inFile.close();
    }
    else {throw BenzeneException() << "Could not open misc-patterns.txt";};

    std::ofstream out;
    out.open("../../../share/ice-patterns.txt"); // TODO : make cleaner
    if (out.is_open())
    {
        out << "\n"
	    << "  File generated (precomputation) from fillin-patterns.txt\n"
	    << "  and misc-patterns.txt.\n";
	if (!deduce_inferior)
	    out << "  Inferior not derived.\n";
	if (!deduce_s_reversible)
	    out << "  Strong reversible not derived.\n";
	if (!iterative_inferior)
	    out << "  Iterative inferior not copied.\n";
	if (!use_big)
	    out << "  Big not used.\n";	  
	out << "\n";

	PatterPrecomputationInfos ppi;
        for (std::size_t i=0; i < fillin.size(); ++i) 
	    AddVariations(fillin[i],out,deduce_s_reversible,
			  deduce_inferior,use_big,ppi) ;
	for (std::size_t i=0; i < misc.size(); ++i) 
	{
	    char type = misc[i].GetType() ;
	    if (!iterative_inferior && Contains(misc[i],"it"))
	    {
		++ppi.it_ignored;
		continue;
	    }
	    switch (type)
	    {
	    case Pattern::EITHER_FILLIN :
	        ++ppi.e_fillin_copied;
		break;
	    case Pattern::WHITE_FILLIN :
	        ++ppi.fillin_copied;
		break;
	    case Pattern::WHITE_STRONG_REVERSIBLE :
	        ++ppi.s_reversible_copied;
		break;
	    case Pattern::WHITE_THREAT_REVERSIBLE :
	        ++ppi.t_reversible_copied;
		break;
	    case Pattern::WHITE_INFERIOR :
	        ++ppi.inferior_copied;
		break;
	    case Pattern::WHITE_REVERSIBLE :
	        ++ppi.reversible_copied;
		break;
	    default :
	        throw BenzeneException() << "Bad type in misc-patterns.txt: "
					 << type ;
	    }
	    out << " [" << misc[i].GetName() << "/]" << std::endl ;
	    out << misc[i].Serialize() << std::endl ;
	}
	out.close();

	std::cout << ppi.e_fillin_copied << " either fillin copied.\n"
		  << ppi.e_fillin_ignored << " either fillin ignored.\n"
		  << ppi.fillin_copied << " fillin copied.\n"
		  << ppi.fillin_ignored << " fillin ignored.\n"
		  << ppi.s_reversible_derived << " strong reversible derived.\n"
		  << ppi.s_reversible_copied << " strong reversible copied.\n"
		  << ppi.inferior_derived << " inferior derived.\n"
		  << ppi.inferior_copied << " inferior copied.\n"
		  << ppi.t_reversible_copied << " threat reversible copied.\n"
		  << ppi.reversible_copied << " reversible copied.\n"
		  << ppi.it_ignored << " iterative ignored.\n"
		  << ppi.big_ignored << " big ignored.\n";
    }
    else {throw BenzeneException() << "Could not open ice-patterns.txt";};
  
    return 0 ;
}

//----------------------------------------------------------------------------
