/*****************************************************************************
 * reductions_pack.h
 *
 ****************************************************************************/

#ifndef REDUCTIONS_PACK_H
#define REDUCTIONS_PACK_H

// local includes
#include "../../../extern/KaHIP/lib/definitions.h"
#include "fast_set.h"
#include "../../data_structure/sized_vector.h"
#include "../../data_structure/dynamic_graph_pack.h"

// system includes 
#include<vector>
#include<memory>
#include<array>
class reduce_algorithm; 

// Update this when more reuctions are implemented.
enum m2ps_reduction_type {clique2, domination2, deg_one2, twin2, cycle2, fast_domination2, neighborhood2, clique_e, domination_e, deg_one_e, twin_e, cycle_e, fast_domination_e, neighborhood_e};  
constexpr size_t m2ps_REDUCTION_NUM = 14; // this is the number of the reductions  

class vertex_marker_2pack {

private:
	sized_vector<NodeID> current; 
	sized_vector<NodeID> next; 
	fast_set added_vertices; 
        

public: 
	vertex_marker_2pack(size_t size) : current(size), next(size), added_vertices(size) {}; 

	void add(NodeID vertex) {
		if(!added_vertices.get(vertex)) {
			next.push_back(vertex); 
			added_vertices.add(vertex); 
		}
	}



	void get_next() {
		if(next.size() != 0) {
			current.swap(next); 
			clear_next();
		}
	}

	void clear_next() {
		next.clear(); 
		added_vertices.clear(); 
	}

	void fill_current_ascending(size_t n) {
		current.clear(); 
		for(size_t i = 0; i < n; i++) {
			current.push_back(i); 
		}
	}

	NodeID current_vertex(size_t index) {
		return current[index]; 
	}

	size_t current_size() {
		return current.size(); 
	}
}; 

struct general_reduction_2pack {
	
	general_reduction_2pack(size_t n) : marker(n) {}
	virtual ~general_reduction_2pack() {}
	virtual general_reduction_2pack* clone() const = 0; 

	virtual m2ps_reduction_type get_reduction_type() const = 0; 
	virtual bool reduce(reduce_algorithm* algo) = 0; 
	virtual void restore(reduce_algorithm* algo) {}
	virtual void apply(reduce_algorithm* algo) {}

	bool has_run = false; 
	vertex_marker_2pack marker; 
}; 

struct deg_one_2reduction : public general_reduction_2pack {
        deg_one_2reduction(size_t n) : general_reduction_2pack(n) {}
	~deg_one_2reduction() {}
	virtual deg_one_2reduction* clone() const final { return new deg_one_2reduction(*this); }

        virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::deg_one2; }
        virtual bool reduce(reduce_algorithm* algo) final; 
}; 

struct cycle2_reduction : public general_reduction_2pack {
        cycle2_reduction(size_t n) : general_reduction_2pack(n) {}
        ~cycle2_reduction() {}
        virtual cycle2_reduction* clone() const final { return new cycle2_reduction(*this); }

        virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::cycle2; }
        virtual bool reduce(reduce_algorithm* algo) final; 
};

struct twin2_reduction : public general_reduction_2pack {
	twin2_reduction(size_t n) : general_reduction_2pack(n) {}
       ~twin2_reduction() {}
        virtual twin2_reduction* clone() const final { return new twin2_reduction(*this); }

        virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::twin2; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct fast_domination2_reduction : public general_reduction_2pack {
        fast_domination2_reduction(size_t n) : general_reduction_2pack(n) {}
        ~fast_domination2_reduction() {}
        virtual fast_domination2_reduction* clone() const final { return new fast_domination2_reduction(*this); }

        virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::fast_domination2; }
        virtual bool reduce(reduce_algorithm* algo) final; 
};

struct neighborhood2_reduction : public general_reduction_2pack {
	
	neighborhood2_reduction(size_t n) : general_reduction_2pack(n) {}
	~neighborhood2_reduction() {}
	virtual neighborhood2_reduction* clone() const final { return new neighborhood2_reduction(*this); }

	virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::neighborhood2; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct domination2_reduction : public general_reduction_2pack {
	domination2_reduction(size_t n) : general_reduction_2pack(n) {}
	~domination2_reduction() {}
	virtual domination2_reduction* clone() const final { return new domination2_reduction(*this); }

	virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::domination2; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct clique2_reduction : public general_reduction_2pack {
	clique2_reduction(size_t n) : general_reduction_2pack(n) {}
	~clique2_reduction() {}
	virtual clique2_reduction* clone() const final { return new clique2_reduction(*this); }

	virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::clique2; }
	virtual bool reduce(reduce_algorithm* algo) final; 
	//virtual void restore(reduce_algorithm* algo) final; 
	//virtual void apply(reduce_algorithm* algo) final; 

private: 
	struct weighted_node {
		NodeID node; 
		NodeWeight weight; 
	};

	struct restore_data {
		weighted_node isolated; 
		std::vector<NodeID> non_isolated; 

		restore_data() = default; 
		restore_data(const weighted_node&isolated, std::vector<NodeID>&& non_isolated) : isolated(isolated), non_isolated(std::move(non_isolated)) {};
	}; 

	void fold(reduce_algorithm* algo, const weighted_node& isolated, std::vector<NodeID>&& non_isolated); 
	
	std::vector<restore_data> restore_vec; 
};

struct deg_one_2reduction_e : public general_reduction_2pack {
        deg_one_2reduction_e(size_t n) : general_reduction_2pack(n) {}
	~deg_one_2reduction_e() {}
	virtual deg_one_2reduction_e* clone() const final { return new deg_one_2reduction_e(*this); }

        virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::deg_one_e; }
        virtual bool reduce(reduce_algorithm* algo) final; 
}; 

struct cycle2_reduction_e : public general_reduction_2pack {
        cycle2_reduction_e(size_t n) : general_reduction_2pack(n) {}
        ~cycle2_reduction_e() {}
        virtual cycle2_reduction_e* clone() const final { return new cycle2_reduction_e(*this); }
        virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::cycle_e; }
        virtual bool reduce(reduce_algorithm* algo) final; 
};

struct twin2_reduction_e : public general_reduction_2pack {
	twin2_reduction_e(size_t n) : general_reduction_2pack(n) {}
       ~twin2_reduction_e() {}
        virtual twin2_reduction_e* clone() const final { return new twin2_reduction_e(*this); }
        virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::twin_e; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct fast_domination2_reduction_e : public general_reduction_2pack {
        fast_domination2_reduction_e(size_t n) : general_reduction_2pack(n) {}
        ~fast_domination2_reduction_e() {}
        virtual fast_domination2_reduction_e* clone() const final { return new fast_domination2_reduction_e(*this); }
        virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::fast_domination_e; }
        virtual bool reduce(reduce_algorithm* algo) final; 
};

struct neighborhood2_reduction_e : public general_reduction_2pack {
	neighborhood2_reduction_e(size_t n) : general_reduction_2pack(n) {}
	~neighborhood2_reduction_e() {}
	virtual neighborhood2_reduction_e* clone() const final { return new neighborhood2_reduction_e(*this); }
	virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::neighborhood_e; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct domination2_reduction_e : public general_reduction_2pack {
	domination2_reduction_e(size_t n) : general_reduction_2pack(n) {}
	~domination2_reduction_e() {}
	virtual domination2_reduction_e* clone() const final { return new domination2_reduction_e(*this); }

	virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::domination_e; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct clique2_reduction_e : public general_reduction_2pack {
	clique2_reduction_e(size_t n) : general_reduction_2pack(n) {}
	~clique2_reduction_e() {}
	virtual clique2_reduction_e* clone() const final { return new clique2_reduction_e(*this); }

	virtual m2ps_reduction_type get_reduction_type() const final { return m2ps_reduction_type::clique_e; }
	virtual bool reduce(reduce_algorithm* algo) final; 

private: 
	struct weighted_node {
		NodeID node; 
		NodeWeight weight; 
	};

	struct restore_data {
		weighted_node isolated; 
		std::vector<NodeID> non_isolated; 

		restore_data() = default; 
		restore_data(const weighted_node&isolated, std::vector<NodeID>&& non_isolated) : isolated(isolated), non_isolated(std::move(non_isolated)) {};
	}; 

	void fold(reduce_algorithm* algo, const weighted_node& isolated, std::vector<NodeID>&& non_isolated); 
	
	std::vector<restore_data> restore_vec; 
};

struct reduction2_ptr {
	general_reduction_2pack* reduction = nullptr; 

	reduction2_ptr() = default; 

	~reduction2_ptr() {
		release(); 
	}

	reduction2_ptr(general_reduction_2pack* reduction) : reduction(reduction) {}; 

	reduction2_ptr(const reduction2_ptr& other) : reduction(other.reduction->clone()) {};

	reduction2_ptr& operator=(const reduction2_ptr& other) {
		release(); 
		reduction = other.reduction->clone(); 
		return *this; 
	}; 

	reduction2_ptr(reduction2_ptr&& other) : reduction(std::move(other.reduction)) {
		other.reduction = nullptr; 
	}; 

	general_reduction_2pack* operator->() const {
		return reduction; 
	}

	void release() {
		if (reduction) {
			delete reduction; 
			reduction = nullptr; 
		}
	}; 
}; 

template<class Last>
void make_2reduction_vector_helper(std::vector<reduction2_ptr>& vec, size_t n) {
	vec.emplace_back(new Last(n)); 
}; 

template<class First, class Second, class ...Redus>
void make_2reduction_vector_helper(std::vector<reduction2_ptr>& vec, size_t n) {
	vec.emplace_back(new First(n)); 
	make_2reduction_vector_helper<Second, Redus...>(vec, n); 
}; 

template<class ...Redus>
std::vector<reduction2_ptr> make_2reduction_vector(size_t n) {
	std::vector<reduction2_ptr> vec; 
	make_2reduction_vector_helper<Redus...>(vec, n); 
	return vec; 
}
#endif // REDUCTIONS_H
