#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <cstring>

#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <utility>
#include <memory>

namespace NRS{
    using namespace std;
    //this is the type we use for signed integers
    typedef long long int si;
    
    const si MODE_OR = 1;
    const si MODE_AND = 0;
    
    typedef si property;
    typedef set<property> propset;
    
    struct property_knowl{
        si propid = 0; //what is the unique id of this property, used in propsets?
        void* src_data = 0;
        void* udata = 0;
    };

    struct thingie {
        propset props = propset();
        void* src_data = 0;
        void* udata = 0;
    };
    //all nodes that have a combination (OR, AND) of the
    //properties will be filtered.
    struct property_filter{
        public: 
            propset props = propset();
            set<property_filter> subfilters = set<property_filter>();
            si mode = MODE_AND;
            si is_inverted = 0;
            void* src_data = 0;
            void* udata = 0;
        //NOTE TO SELF:
        //not (property) can be represented through a subfilter.
        /*
            property_filter
                property_filter
                    is_inverted = 1;
                    property 1
                    AND
        */
        private:
            int matches_inner(thingie* t) const{
                if(mode == MODE_AND){//must contain all properties and match all subfilters.
                    for(auto prop: props)
                        if(t->props.count(prop) == 0)
                            return 0;
                    for (auto& filt: subfilters)
                        if(!filt.matches(t))
                            return 0;
                    return 1;
                }else{ //can match any property or any subfilter.
                    for(auto prop: props)
                        if(t->props.count(prop) == 1)
                            return 1;
                    for (auto& filt: subfilters)
                        if(filt.matches(t))
                            return 1;
                    return 0;
                }
                return 0;
            }
        public: 
            int matches(thingie* t) const {
                if(is_inverted) return !matches_inner(t);
                return !!matches_inner(t);
            }
    };
    struct constraint{
        property_filter criterion;
        void* src_data = 0;
        void* udata = 0;
        int apply(thingie* t){
            return criterion.matches(t);
        }
    };
    //this is how new properties are added to a node:
    //if something has a certain property, we can
    //add new properties to it.
    struct propagator{
        property_filter criterion;
        propset implied;
        void* src_data = 0;
        void* udata = 0;
        int apply(thingie* t){
            int made_change = 0;
            if(criterion.matches(t))
                for(auto prop: implied)
                    if(t->props.count(prop) == 0)
                    {
                        made_change = 1;    
                        t->props.insert(prop);
                    }
            return made_change;
        }
    };

    struct rule{
        string rulename; //what is the name of the rule, if it has one?
        void* src_data = 0;
        void* udata = 0;
        propset new_properties; //what properties does this rule define? Used for debugging.
        vector<thingie> new_thingies; //what new thingies does this rule add?
        vector<propagator> new_propagators; //what new propagators does this rule add?
        vector<constraint> new_constraints; //what new constraints does this rule add?
    };
    
    //the thingie-property system.
    struct propsystem{
        vector<propagator> propagators;
        vector<constraint> constraints;
        vector<thingie> thingies;
        vector<rule*> applied_rules;
        void* src_data = 0;
        void* udata = 0;
        si failed = 0;
        si failed_maxiter = 0;
        int iteration(){
            if(failed) return 2;
            si made_change = 0;
            for(auto& t: thingies){
                for(auto& p: propagators)
                    made_change |= p.apply(&t);
                for(auto& c: constraints)
                    if(c.apply(&t)){
                        failed = 1;
                        return made_change | 2;
                    }
            }
            return made_change;
        }
        //returns 1 on success to apply the rule.
        //returns 0 on failure to apply the rule, or already failed.
        int apply_rule(rule* r, si max_iterations){
            int iter_retval;
            si iteration_count = 0;
            if(failed) return 0;
            for(auto& q: r->new_propagators)
                propagators.push_back(q);
            for(auto& q: r->new_constraints)
                constraints.push_back(q);
            for(auto& q: r->new_thingies)
                thingies.push_back(q);
            while(1){
                if(iteration_count > max_iterations) {
                    failed_maxiter = 1;
                    failed = 1;
                    return 0;
                }
                iter_retval = iteration();
                iteration_count++;
                if(iter_retval & 2) return 0; //failure!
                if(iter_retval & 1) continue; //a change was made.
                break;
            }
        }
    };
    
    

}

using namespace NRS;

int main(int argc, char** argv){
    (void)argc;
    (void)argv;

}
