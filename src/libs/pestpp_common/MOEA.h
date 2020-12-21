#ifndef MOEA_H_
#define MOEA_H_

#include <unordered_map>
#include <random>
#include <vector>
#include <utility>
#include <iostream>
#include <fstream>
#include <string>
#include "FileManager.h"
#include "ObjectiveFunc.h"
#include "OutputFileWriter.h"
#include "PerformanceLog.h"
#include "Ensemble.h"
#include "constraints.h"
#include "EnsembleMethodUtils.h"

const string POP_SUM_TAG = "pareto.summary.csv";
const string ARC_SUM_TAG = "pareto.archive.summary.csv";
const string RISK_NAME = "_RISK_";

class ParetoObjectives
{
public:
	ParetoObjectives(Pest& _pest_scenario, FileManager& _file_manager, 
		PerformanceLog* _performance_log);

	pair<vector<string>, vector<string>> nsga_ii_pareto_dominance_sort(int generation, ObservationEnsemble& op, 
		ParameterEnsemble& dp, Constraints* constraints_ptr=nullptr, bool report=true, string sum_tag=string());
	
	//this must be called at least once before the diversity metrixs can be called...
	void set_pointers(vector<string>& _obs_obj_names, vector<string>& _pi_obj_names, map<string, double>& _obj_dir_mult)
	{
		obs_obj_names_ptr = &_obs_obj_names; 
		pi_obj_names_ptr = &_pi_obj_names; 
		obj_dir_mult_ptr = &_obj_dir_mult;
		prep_pareto_summary_file(POP_SUM_TAG);
		prep_pareto_summary_file(ARC_SUM_TAG);
	}
	

	void update(ObservationEnsemble& oe, ParameterEnsemble& dp, Constraints* constraints_ptr = nullptr);

	bool compare_two(string& first, string& second);

	map<string, double> get_spea2_fitness(int generation, ObservationEnsemble& op, ParameterEnsemble& dp, 
		Constraints* constraints_ptr = nullptr, bool report = true, string sum_tag = string());
	map<string, double> get_kth_nn_crowding_distance(ObservationEnsemble& oe, ParameterEnsemble& dp);
	 
	void get_spea_names_to_keep(int num_members, vector<string>& keep, const ObservationEnsemble& op, const ParameterEnsemble& dp);

private:
	
	Pest& pest_scenario;
	FileManager& file_manager;
	PerformanceLog* performance_log;
	//vector<string> obj_names;
	vector<string> sort_members_by_crowding_distance(vector<string>& members, map<string, double>& crowd_map, map<string, map<string, double>>& _member_struct);
	bool first_dominates_second(map<string, double>& first, map<string, double>& second);
	map<string, map<string, double>> get_member_struct(ObservationEnsemble& oe, ParameterEnsemble& dp);
	void drop_duplicates(ObservationEnsemble& op, ParameterEnsemble& dp, map<string, map<string, double>>& _member_struct);
	bool first_equals_second(map<string, double>& first, map<string, double>& second);

	map<int, vector<string>> sort_members_by_dominance_into_fronts(map<string, map<string, double>>& _member_struct);
	map<string,double> get_spea_fitness(map<string, map<string, double>>& _member_struct);

	void fill_domination_containers(map<string, map<string, double>>& _member_struct, map<string,
		vector<string>>&solutions_dominated_map, map<string, int>& num_dominating_map, bool dup_as_dom=false);
	

	bool compare_two_nsga(string& first, string& second);

	//sort specific members
	map<string, double> get_cuboid_crowding_distance(vector<string>& members, map<string, map<string, double>>& _member_struct);
	//sort all members in member struct
	//map<string, double> get_cuboid_crowding_distance();
	map<string, double> get_cuboid_crowding_distance(map<string, map<string, double>>& _member_struct);


	map<string, double> get_kth_nn_crowding_distance(map<string, map<string, double>>& _member_struct);
	map<string, double> get_kth_nn_crowding_distance(vector<string>& members, map<string, map<string, double>>& _member_struct);

	
	map<string, double> get_cuboid_crowding_distance(ObservationEnsemble& oe, ParameterEnsemble& dp);

	

	map<string, map<string, double>> member_struct;
	vector<string>* obs_obj_names_ptr;
	vector<string>* pi_obj_names_ptr;
	map<string, double>* obj_dir_mult_ptr;

	map<string, map<string, double>> feas_member_struct;
	map<int, vector<string>> front_map;
	map<string, double> crowd_map;
	map<string, int> member_front_map;
	map<string, double> infeas;
	vector<string> infeas_ordered;
	map<string, double> fitness_map;

	
	
	void prep_pareto_summary_file(string summary_tag);
	void write_pareto_summary(string& sum_tag, int generation, vector<string>& member_names,
		map<string, int>& front_map, map<string, double>& crowd_map, map<string, double>& infeas_map,
		map<string, map<string, double>>& _member_struct);
	
};


class MOEA
{
	enum MouGenType{DE,SBX};
	enum MouEnvType { NSGA, SPEA };
public:
	static mt19937_64 rand_engine;
	MOEA(Pest &_pest_scenario, FileManager &_file_manager, OutputFileWriter &_output_file_writer,
		PerformanceLog *_performance_log, RunManagerAbstract* _run_mgr_ptr);
	void initialize();
    void iterate_to_solution();
	void finalize();
	typedef pair<vector<string>, vector<string>> DomPair;
private:
	MouEnvType envtype;
	double epsilon = 1.0e-15;
	Pest& pest_scenario;
	set<string> pp_args;
	vector<MouGenType> gen_types;
	vector<string> act_obs_names, act_par_names;
	int iter, warn_min_members, error_min_members;
	int member_count;
	int archive_size;
	string population_dv_file, population_obs_restart_file;
	string dv_pop_file_tag = "dv_pop";
	string obs_pop_file_tag = "obs_pop";
	string lineage_tag = "lineage.csv";
	chancePoints chancepoints;
	FileManager &file_manager; 
	std::mt19937 rand_gen;
	vector<string> obs_obj_names, pi_obj_names;
	vector<string> dv_names;
	map<string, double> obj_dir_mult;

	map<string, map<string, double>> previous_obj_summary;
	bool risk_obj;

	//these two instances are passed as pointers to the constraints
	//Parameters effective_constraint_pars;
	//Observations effective_constraint_obs;

	ParetoObjectives objectives;
	Constraints constraints;
	const ParameterInfo *ctl_par_info_ptr;
	const ParameterGroupInfo *par_group_info_ptr;
	ParamTransformSeq par_transform;
	OutputFileWriter &output_file_writer;
	PerformanceLog *performance_log;
	RunManagerAbstract* run_mgr_ptr;
	const ObservationInfo *obs_info_ptr;

	ParameterEnsemble dp, dp_archive;
	ObservationEnsemble op, op_archive;

	void update_archive_nsga(ObservationEnsemble& _op, ParameterEnsemble& _dp);
	void update_archive_spea(ObservationEnsemble& _op, ParameterEnsemble& _dp);

	void throw_moea_error(const string& message);

	template<typename T, typename A>
	void message(int level, const string& _message, vector<T, A> _extras, bool echo = true);
	void message(int level, const string& _message);
	template<typename T>
	void message(int level, const string& _message, T extra);

	void sanity_checks();
	vector<int> run_population(ParameterEnsemble& _dp, ObservationEnsemble& _op, bool allow_chance);

	void queue_chance_runs(ParameterEnsemble& _dp);
	ObservationEnsemble get_chance_shifted_op(ParameterEnsemble& _dp, ObservationEnsemble& _op);

	bool initialize_dv_population();
	void initialize_obs_restart_population();

	ParameterEnsemble generate_population();

	ParameterEnsemble generate_diffevol_population(int num_members, ParameterEnsemble& _dp);
	ParameterEnsemble generate_sbx_population(int num_members, ParameterEnsemble& _dp);
	ParameterEnsemble generate_pm_population(int num_members, ParameterEnsemble& _dp);

	vector<int> selection(int num_to_select, ParameterEnsemble& _dp, bool use_binary_tourament);

	string get_new_member_name(string tag = string());

	void save_populations(ParameterEnsemble& dp, ObservationEnsemble& op, string tag = string());
	void linear_mutation_ip(double probability, double eta_m, ParameterEnsemble& temp_dp);
	pair<Eigen::VectorXd, Eigen::VectorXd> sbx(double probability, double eta_m, int idx1, int idx2);
	pair<Eigen::VectorXd, Eigen::VectorXd> sbx_new(double probability, double eta_m, int idx1, int idx2);
	pair<double, double> get_betas(double v1, double v2, double distribution_index);

	pair<Parameters, Observations> get_optimal_solution(ParameterEnsemble& _dp, ObservationEnsemble& _oe);

	map<string, map<string, double>> obj_func_report(ParameterEnsemble& _dp, ObservationEnsemble& _oe);
	map<string, map<string, double>> get_obj_func_summary_stats(ParameterEnsemble& _dp, ObservationEnsemble& _op);
	map<string, map<string, double>> obj_func_change_report(map<string, map<string, double>>& current_obj_summary);

	int get_max_len_obj_name();
};

#endif //MOEA_H_
