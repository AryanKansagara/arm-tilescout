#include "tilescout.hpp"
#include <iostream>
int main(int argc,char**argv){if(argc>2||(argc==2&&std::string(argv[1])!="demo")){std::cerr<<"usage: tilescout [demo]\n";return 1;}auto d=tile::evaluate();std::cout<<"{\"kind\":\"portable_packing_contract_fixture\",\"cases\":"<<d.cases<<",\"reference_matches\":"<<d.matches<<",\"seeded_tail_defects_detected\":"<<d.tail_defects<<",\"arm_instructions_executed\":false,\"injected_capability_policy_only\":true}\n";return d.matches==d.cases?0:1;}
