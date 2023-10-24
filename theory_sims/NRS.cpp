/*
    NETWORKED REASONING SOLVER THEORY TESTBENCH.

*/


#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <utility>

#define MAX_COORDS 4
/*
    
*/





using namespace std;
namespace NRS{
using namespace std;
    class coords{
        public:
            int d[MAX_COORDS];
        coords(){
            for(unsigned long i = 0; i < MAX_COORDS; i++)
                d[i] = 0;
        }
        int equals(coords& other, int good_dims){
            for(int i = 0; i < good_dims && i < MAX_COORDS; i++)
                if(d[i] != other.d[i])
                    return 0;
            return 1;
        }
    };
    class thingie {
        private:
            int change_validator = 0;
        public:
        void* udata = NULL;
        
        vector<int> properties; //
        int has_property(int in){
            for(auto& qq: properties)
                if(qq == in) return 1;
            return 0;
        }
        //push_property and remove_property return whether a change was made.
        int push_property(int in){
            if(has_property(in)) return 0;
            properties.push_back(in);
            change_validator = 1;
            return 1;
        }
        int remove_property(int in){
            for(unsigned long i = 0; i < properties.size(); i++)
                if(properties[i] == in){
                    properties.erase(properties.begin() + i);
                    change_validator = 1;
                    return 1;
                }
            return 0;
        }
        typedef int (*thingie_constraint_function)(thingie&); //returns whether the constraint caused a rejection
        typedef void (*thingie_property_propagator)(thingie&); //returns whether a change was made to properties.
        
        //constraint functions return whether a contradiction was found.
        //propagators simply make changes to properties.
        int try_constraint(thingie_constraint_function f){
            return f(*this);
        }
        int try_propagator(thingie_property_propagator f){
            change_validator = 0;
            f(*this);
            return (change_validator == 0);
        }
    };
    class relationship_entry{
        public:
            thingie* me; //TODO: implement "thingie"
            void* udata = NULL;
            coords c;
        //evaluate for equality.
    };
    
    //needed for relationship property propagation.
    pair<bool,pair<thingie*, int>> gen_prop_change(
        bool is_adding, 
        thingie* targ, 
        int prop
    ){
        return std::make_pair(is_adding, std::make_pair(targ, prop));
    }
    
    class relationship{
        public:
            int relationship_type = 0;
            //how many coordinates are relevant?
            //by default, use only 1
            int relationship_relevant_coords = 1;
            //can't use set because
            //we don't have operator==
            //for members.
            vector<relationship_entry> members = vector<relationship_entry>();
        //check if entry exists for thingie
            int check_if_entry_exists(thingie* me){
                for(auto& qq : members){
                    if(qq.me == me)
                        return 1;
                }
                return 0;
            }

        //push a member. Returns whether or not it succeeded in adding the entry.
            int push_entry(thingie* me, coords c){
                if(check_if_entry_exists(me)) return 0;
                relationship_entry e;
                e.me = me;
                e.c = c;
                members.push_back(e);
                return 1;
            }
        //constraints return 1 on failure.
            typedef int (*relationship_constraint_function)(relationship&);
            //relationship property propagators cause properties to be added or removed.
            typedef vector //vector of...
                <
                    pair< //pairs of
                        bool, //should add?
                        pair<thingie*, int> //pair of thingie* and int
                    >
                > (*property_propagator_function)(relationship&);
            static pair<bool,pair<thingie*, int>> prop_change(
                bool is_adding,
                thingie* targ, 
                int prop
            ){
                return std::make_pair(is_adding, std::make_pair(targ, prop));
            }
        //Apply a property propagator.
            int try_propagator(
                property_propagator_function f
            ){
                int made_change = 0;
                auto props = f(*this);
                for(auto& newprop: props)
                    for(auto& entry: members)
                    {
                        thingie* me = std::get<thingie*>(
                            std::get<pair<thingie*, int>>(newprop)
                        );
                        if(me != entry.me) continue;
                        bool should_add = std::get<bool>(newprop);
                        int prop_to_add = std::get<int>(std::get<pair<thingie*, int>>(newprop));
                        if(should_add){
                            made_change |= (me->push_property(prop_to_add) != 0);
                        } else {
                            made_change |= (me->remove_property(prop_to_add) != 0);
                        }
                    }
                return made_change;
            }
        
            int try_rel_constraint(
                relationship_constraint_function f
            ){
                return f(*this);
            }
            
            int try_thingie_constrait(
                thingie::thingie_constraint_function f
            ){
                int failed = 0;
                for(auto& qq: members)
                    failed |= f(qq.me[0]);
                return failed;
            }
    };
    
    class NRS_System{
        public:
        vector<relationship> relationships;
        vector<thingie> thingies;
    };
    
    void imply_self(
        thingie& t,
        vector<int>& implicators,
        vector<int>& implicated
    ){
        for(auto& cause: implicators)
            if(t.has_property(cause))
                for(auto& effect: implicated)
                    t.push_property(effect);
    }
    
    int mutually_exclusive_constraint_self(
        thingie& t,
        vector<int>& exclusive_props
    ){
        for(auto& p1: exclusive_props)
            for(auto& p2: exclusive_props)
            {
                if(p1 == p2) continue;
                if(t.has_property(p1))
                    if(t.has_property(p2))
                        return 1;
            }
        return 0;
    }
    
    /*
        TODO:
        * Relationship (coords) implies property on a thingie
        * Relationship (coords) + properties implies property on a thingie
    */
    


};

using namespace NRS;

int main(int argc, char** argv){
    (void)argc;
    (void)argv;

}



