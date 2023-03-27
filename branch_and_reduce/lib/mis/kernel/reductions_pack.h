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
enum reduction_type {clique, domination, deg_one, twin, cycle, fast_domination, neighborhood};  
constexpr size_t REDUCTION_NUM = 2; // this is the number of the reductions  

class vertex_marker {

private:
	sized_vector<NodeID> current; 
	sized_vector<NodeID> next; 
	fast_set added_vertices; 

public: 
	vertex_marker(size_t size) : current(size), next(size), added_vertices(size) {}; 

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

struct general_reduction {
	
	general_reduction(size_t n) : marker(n) {}
	virtual ~general_reduction() {}
	virtual general_reduction* clone() const = 0; 

	virtual reduction_type get_reduction_type() const = 0; 
	virtual bool reduce(reduce_algorithm* algo) = 0; 
	virtual void restore(reduce_algorithm* algo) {}
	virtual void apply(reduce_algorithm* algo) {}

	bool has_run = false; 
	vertex_marker marker; 
}; 

struct deg_one_reduction : public general_reduction {
        deg_one_reduction(size_t n) : general_reduction(n) {}
	~deg_one_reduction() {}
	virtual deg_one_reduction* clone() const final { return new deg_one_reduction(*this); }

        virtual reduction_type get_reduction_type() const final { return reduction_type::deg_one; }
        virtual bool reduce(reduce_algorithm* algo) final; 
}; 

struct cycle_reduction : public general_reduction {
        cycle_reduction(size_t n) : general_reduction(n) {}
        ~cycle_reduction() {}
        virtual cycle_reduction* clone() const final { return new cycle_reduction(*this); }

        virtual reduction_type get_reduction_type() const final { return reduction_type::cycle; }
        virtual bool reduce(reduce_algorithm* algo) final; 
};

struct twin_reduction : public general_reduction {
	twin_reduction(size_t n) : general_reduction(n) {}
       ~twin_reduction() {}
        virtual twin_reduction* clone() const final { return new twin_reduction(*this); }

        virtual reduction_type get_reduction_type() const final { return reduction_type::twin; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct fast_domination_reduction : public general_reduction {
        fast_domination_reduction(size_t n) : general_reduction(n) {}
        ~fast_domination_reduction() {}
        virtual fast_domination_reduction* clone() const final { return new fast_domination_reduction(*this); }

        virtual reduction_type get_reduction_type() const final { return reduction_type::fast_domination; }
        virtual bool reduce(reduce_algorithm* algo) final; 
};

struct neighborhood_reduction : public general_reduction {
	
	neighborhood_reduction(size_t n) : general_reduction(n) {}
	~neighborhood_reduction() {}
	virtual neighborhood_reduction* clone() const final { return new neighborhood_reduction(*this); }

	virtual reduction_type get_reduction_type() const final { return reduction_type::neighborhood; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct domination_reduction : public general_reduction {
	domination_reduction(size_t n) : general_reduction(n) {}
	~domination_reduction() {}
	virtual domination_reduction* clone() const final { return new domination_reduction(*this); }

	virtual reduction_type get_reduction_type() const final { return reduction_type::domination; }
	virtual bool reduce(reduce_algorithm* algo) final; 
};

struct clique_reduction : public general_reduction {
	clique_reduction(size_t n) : general_reduction(n) {}
	~clique_reduction() {}
	virtual clique_reduction* clone() const final { return new clique_reduction(*this); }

	virtual reduction_type get_reduction_type() const final { return reduction_type::clique; }
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


struct reduction_ptr {
	general_reduction* reduction = nullptr; 

	reduction_ptr() = default; 

	~reduction_ptr() {
		release(); 
	}

	reduction_ptr(general_reduction* reduction) : reduction(reduction) {}; 

	reduction_ptr(const reduction_ptr& other) : reduction(other.reduction->clone()) {};

	reduction_ptr& operator=(const reduction_ptr& other) {
		release(); 
		reduction = other.reduction->clone(); 
		return *this; 
	}; 

	reduction_ptr(reduction_ptr&& other) : reduction(std::move(other.reduction)) {
		other.reduction = nullptr; 
	}; 

	general_reduction* operator->() const {
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
void make_reduction_vector_helper(std::vector<reduction_ptr>& vec, size_t n) {
	vec.emplace_back(new Last(n)); 
}; 

template<class First, class Second, class ...Redus>
void make_reduction_vector_helper(std::vector<reduction_ptr>& vec, size_t n) {
	vec.emplace_back(new First(n)); 
	make_reduction_vector_helper<Second, Redus...>(vec, n); 
}; 

template<class ...Redus>
std::vector<reduction_ptr> make_reduction_vector(size_t n) {
	std::vector<reduction_ptr> vec; 
	make_reduction_vector_helper<Redus...>(vec, n); 
	return vec; 
}

#endif // REDUCTIONS_H
