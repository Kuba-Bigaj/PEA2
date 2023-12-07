#include <iostream>
#include <fstream>
#include <string>
#include <conio.h>
#include <chrono>
#include <vector>

class App {
	//data storage
	std::string loaded_file = "None";
	int** matrix = nullptr;
	int size = 0;

	//algorith-specific data storage
	int run_limit = 20;				//in seconds

	//SA
	std::string cooling_schedules[3] = { "Geometric", "Logarithmic", "Linear" };
	int chosen_cooling_schedule = 0;
	double cooling_coefficient = 0.9999;
	//Geometric		: 0.9999,	0.9999,	0.999
	//Logarithmic	: 0.001,	0.001,	0.01
	//Linear		: 1e-6 ,	1e-5,	5e-5

	//TS
	std::string neighbour_defs[3] = { "Swap", "Insert", "Invert" };
	int chosen_definition = 0;


	//static UI helper functions
	static int create_sub_menu(std::string top_banner, std::string options[], std::string bot_banner, int number, int def_option) // displays a submenu with options, returns the number chosen
	{
		int chosen_option = def_option;
		while (true)
		{
			system("cls");
			std::cout << top_banner;
			for (int i = 0; i < number; i++)
			{
				if (i == chosen_option)
				{
					std::cout << "==>";
				}
				std::cout << "\t" << options[i] << "\n";
			}
			std::cout << bot_banner;
			switch (_getch())
			{
			case 72:
				chosen_option--;
				break;
			case 80:
				chosen_option++;
				break;
			case '\r':
				return chosen_option;
			default:
				break;
			}
			chosen_option = (chosen_option < 0 ? 0 : (chosen_option >= number ? number - 1 : chosen_option));
		}
	}

	static int create_adaptive_sub_menu(std::string top_banner, std::string options[], std::string bot_banner[], int number, int def_option) // displays a submenu with options, returns the number chosen
	{
		int chosen_option = def_option;
		while (true)
		{
			system("cls");
			std::cout << top_banner;
			for (int i = 0; i < number; i++)
			{
				if (i == chosen_option)
				{
					std::cout << "==>";
				}
				std::cout << "\t" << options[i] << "\n";
			}
			std::cout << bot_banner[chosen_option];
			switch (_getch())
			{
			case 72:
				chosen_option--;
				break;
			case 80:
				chosen_option++;
				break;
			case '\r':
				return chosen_option;
			default:
				break;
			}
			chosen_option = (chosen_option < 0 ? 0 : (chosen_option >= number ? number - 1 : chosen_option));
		}
	}

	static bool test_input_validity(std::string err_message)
	{
		if (std::cin.fail())
		{
			std::cout << err_message << "\n";
			std::cin.clear();
			std::cin.ignore(1000, '\n');
			system("pause");
			return false;
		}
		return true;
	}

	//data manipulation functions
	void dealloc_matrix() //deallocates the matrix, sets size to 0
	{
		if (!matrix)
		{
			size = 0;
			return;
		}
		for (int i = 0; i < size; i++)
		{
			delete matrix[i];
		}
		delete matrix;
		matrix = nullptr;
		size = 0;
	}

	void alloc_matrix(int s) //allocates the matrix
	{
		size = s;
		matrix = new int*[s];
		for (int i = 0; i < s; i++)
		{
			matrix[i] = new int[s];
		}
	}

	void show_data()
	{
		std::cout << "Loaded file: " << loaded_file << "\nData size: " << size << "\n";
		system("pause");
	}

	std::string str_path(int* solution)
	{
		std::string output;
		output += std::to_string(solution[0]);
		for (int i = 1; i < size; i++)
		{
			output += " -> " + std::to_string(solution[i]);
		}
		output += "\n";
		return output;
	}

	void read_data_from_file(std::string filename) //TODO adapt
	{
		std::ifstream file(filename);
		std::string line;

		if (!file.is_open())
		{
			std::cout << "File open error!\n";
			system("pause");
			return;
		}

		std::getline(file, line);	//title

		loaded_file = line.substr(6, line.size());

		std::getline(file, line);	//type
		std::getline(file, line);	//comment
		std::getline(file, line);	//dimension

		int f_size = std::stoi(line.substr(11, line.size() - 1));

		std::getline(file, line);	//edge type
		std::getline(file, line);	//edge format
		std::getline(file, line);	//edge section



		dealloc_matrix();

		size = f_size;

		alloc_matrix(size);

		int val;
		for (int i = 0; i < size; i++)
		{
			for (int j = 0; j < size; j++)
			{
				file >> val;
				if (file.fail())
				{
					std::cout << "Data read error!\n";
					break;
				}
				else
					matrix[i][j] = val;
			}
		}
		file.close();
		return;
	}

	void load_data()
	{
		std::string filename;
		std::cout << "Enter filename:\n";
		std::cin >> filename;
		if (test_input_validity("unsupported filename!\n"))
		{
			read_data_from_file(filename);
			std::cout << "Loaded data: \n";
			show_data();
		}
	}

	void copy_arr(int* src, int* dest)
	{
		for (int i = 0; i < size; i++)
		{
			dest[i] = src[i];
		}
	}

	void save_path_to_file(int* path)
	{
		std::ofstream file(loaded_file + "_results.txt");
		file << size << "\n";
		for (int i = 0; i < size; i++)
		{
			file << path[i] << " ";
		}
		file.close();
	}

	//generic algorithm helpers

	int path_len(int* solution)
	{
		int len = 0;
		for (int i = 1; i < size; i++)
		{
			len += matrix[solution[i - 1]][solution[i]];
		}
		len += matrix[solution[size - 1]][solution[0]];
		return len;
	}

	double generate_random_double()
	{
		return (double)((double)rand() / (double)RAND_MAX);
	}

	//returns a new array containing a random solution
	int* generate_random_path()
	{
		int* solution = new int[size];
		bool* vis = new bool[size];
		int cur_v = 0;

		for (int i = 0; i < size; i++)
			vis[i] = false;

		for (int i = 0; i < size; i++)
		{
			cur_v = rand() % size;
			while (vis[cur_v])
			{
				cur_v = (cur_v + 1) % size;
			}
			vis[cur_v] = true;
			solution[i] = cur_v;
		}

		return solution;
	}

	//SA algoritm helpers
	void random_swap(int* solution)
	{
		int one = rand() % size;
		int two = rand() % size;
		while (two == one)
			two = rand() % size;

		std::swap(solution[one], solution[two]);
	}

	void SA_generate_neighbour(int* src, int* dest)
	{
		copy_arr(src, dest);
		random_swap(dest);
	}

	void SA_cool(double& temp)
	{
		switch (chosen_cooling_schedule)
		{
		case 0:
			temp *= cooling_coefficient;
			break;
		case 1:
			temp /= (1 + cooling_coefficient * temp);
			break;
		case 2:
			temp -= cooling_coefficient;
			temp = temp < 0 ? 0 : temp;
			break;
		default:
			break;
		}
	}

	double SA_generate_starting_temp() //calculates the initial temperature based on average diffrences between neighbours
	{
		int *sol1, *sol2 = new int[size];
		double avg_diff = 0.0;
		for (int i = 0; i < 1000; i++)
		{
			sol1 = generate_random_path();
			copy_arr(sol1, sol2);
			random_swap(sol2);

			//std::cout << "Len1: " << path_len(sol1) << "\nPath1: " << str_path(sol1) << "Len2: " << path_len(sol2) << "\nPath2: " << str_path(sol2) << "\n" << abs(path_len(sol1) - path_len(sol2)) << "\n\n\n";
			avg_diff += abs(path_len(sol1) - path_len(sol2));

			delete sol1;
		}
		delete sol2;
		avg_diff /= 1000;
		//std::cout << "diff: " << avg_diff << "\ntemp: " << avg_diff / 0.223 << "\n";
		return avg_diff / 0.223; //e^(-(a-b)/temp)=x => temp=(a-b)/(log((x)^-1)) , x=0.99
	}

	//TS algorithm helpers

	//swap a and b
	void TS_mn_swap(int * solution, int a, int b)
	{
		std::swap(solution[a], solution[b]);
	}

	//insert from a into b
	void TS_mn_insert(int * solution, int a, int b)
	{
		int s_val = solution[a];
		if (a < b)
		{
			for (int i = a; i < b; i++)
			{
				solution[i] = solution[i + 1];
			}
			solution[b] = s_val;
		}
		if (a > b)
		{
			for (int i = a; i > b; i--)
			{
				solution[i] = solution[i - 1];
			}
			solution[b] = s_val;
		}
	}

	//iverts the solution between a and b, inclusive
	void TS_mn_invert(int * solution, int a, int b)
	{
		int min = a < b ? a : b;
		int max = a > b ? a : b;

		int* copy = new int[size];
		copy_arr(solution, copy);

		for (int i = 0; i <= max - min; i++)
		{
			solution[min + i] = copy[max - i];
		}

		delete copy;
	}

	void TS_generate_neighbour(int * solution, int a, int b)
	{
		switch (chosen_definition)
		{
		case 0:
			TS_mn_swap(solution, a, b);
			break;
		case 1:
			TS_mn_insert(solution, a, b);
			break;
		case 2:
			TS_mn_invert(solution, a, b);
			break;
		default:
			break;
		}
	}

	//path genration algorithms
	int lower_bound()
	{
		int res = 0, min;
		for (int i = 0; i < size; i++)
		{
			min = INT_MAX;
			for (int j = 0; j < size; j++)
			{
				if (i == j)
					continue;
				min = min < matrix[i][j] ? min : matrix[i][j];
			}
			res += min;
		}
		return res;
	}

	int higher_bound()
	{
		int res = 0, max;
		for (int i = 0; i < size; i++)
		{
			max = INT_MIN;
			for (int j = 0; j < size; j++)
			{
				if (i == j)
					continue;
				max = max > matrix[i][j] ? max : matrix[i][j];
			}
			res += max;
		}
		return res;
	}

	int* greedy()
	{
		int* solution = new int[size];
		bool* vis = new bool[size];
		bool is_done = false;
		int cur_vert, min, min_v;

		cur_vert = 0;
		for (int i = 0; i < size; i++)
			vis[i] = false;

		for (int c = 0; !is_done; c++)
		{
			solution[c] = cur_vert;
			vis[cur_vert] = true;

			is_done = true;
			min = INT_MAX;
			min_v = 0;

			for (int i = 0; i < size; i++)
			{
				if (vis[i])
					continue;

				is_done = false;

				if (matrix[cur_vert][i] < min)
				{
					min = matrix[cur_vert][i];
					min_v = i;
				}
			}
			cur_vert = min_v;
		}
		return solution;
	}

	void simulated_annealing()
	{
		if (size == 0)
		{
			std::cout << "No data loaded!\n";
			system("pause");
			return;
		}

		bool should_stop = false;
		int* s1 = greedy();
		int* s2 = new int[size];
		int* s3 = new int[size];
		int l1, l2, l3 = INT_MAX;
		//https://courses.physics.illinois.edu/phys466/sp2013/projects/2001/team1/cooling.htm Basically, a good starting point is a temeprature, which gives the acceptance of 80% => exp(-(l2-l1).temp) = 0.8
		double temp = SA_generate_starting_temp(); //I chose an acceptace rate of 99%

		std::cout << "Starting temperature:" << temp << "\n\n";

		SA_generate_neighbour(s1, s2);

		int old_sol_len = 0, old_sol_cnt = 0, disp_cnt = 0; //misc

		auto start = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed;

		while (!should_stop)
		{
			for (int i = 0; i < size * 4; i++)
			{
				l1 = path_len(s1);
				l2 = path_len(s2);

				if (l1 < l3) //global minimum solutiom check
				{
					copy_arr(s1, s3);
					l3 = l1;
					std::cout << "IMPROVED: "<<elapsed.count() <<"\n";
				}

				if (l1 > l2)
					copy_arr(s2, s1);
				else
				{
					double rand_dob = generate_random_double();
					double exp_wyr = exp((-(l2 - l1)) / temp);

					//std::cout << "exp:\t"<<exp_wyr<<"\n";

					if (rand_dob < exp_wyr)
						copy_arr(s2, s1);
				}

				SA_generate_neighbour(s1, s2);

			}

			l1 = path_len(s1);

			SA_cool(temp);

			/*
			disp_cnt++;
			if (disp_cnt == 1000)
			{
				std::cout << "len:" << l1 << "\ttemp:" << temp << "\n";
				disp_cnt = 0;
			}
			*/
			elapsed = std::chrono::system_clock::now() - start;

			if (old_sol_len == l1)
				old_sol_cnt++;
			else
				old_sol_cnt = 0;

			if (elapsed.count() > run_limit || (temp < 0.1 && old_sol_cnt == 50))
				should_stop = true;
			old_sol_len = l1;
		}
		std::cout << "Time elapsed: " << elapsed.count() << "s\nFinal temperature:" << temp << "\ne^(-1/temp): " << exp(-1 / temp) << "\nFinal path length: " << l3 //<< "\n";
			<< "\nFinal path:\n" << str_path(s3) << "\n";
		save_path_to_file(s3);
		delete s1;
		delete s2;
		delete s3;
	}

	void taboo_search()
	{
		if (size == 0)
		{
			std::cout << "No data loaded!\n";
			system("pause");
			return;
		}

		int* global_best_solution = greedy();
		//int* global_best_solution = generate_random_path();
		int* near_best_solution = new int[size];
		int* current_solution = new int[size];
		int* current_neighbour = new int[size];
		int* cur_move = new int[2];

		copy_arr(global_best_solution, current_solution);

		int global_best_cost = path_len(global_best_solution);
		int near_best_cost;
		int neighbour_cost;

		int it_counter = 0;

		int** taboo_list = new int*[size];
		for (int i = 0; i < size; i++)
		{
			taboo_list[i] = new int[size];
			for (int j = 0; j < size; j++)
			{
				taboo_list[i][j] = 0;
			}
		}

		auto start = std::chrono::system_clock::now();
		std::chrono::duration<double> elapsed;

		do {
			it_counter++;
			if (it_counter == 1000)
			{
				for (int i = 0; i < 10; i++)
					random_swap(current_solution);
				continue;
			}

			near_best_cost = INT_MAX;
			cur_move[0] = 0;
			cur_move[1] = 0;



			for (int i = 0; i < size; i++)
			{
				for (int j = 0; j < size; j++)
				{
					if (i == j)
					{
						continue;
					}


					if (taboo_list[i][j] != 0)
					{
						if (generate_random_double() > 1.0 / taboo_list[i][j])
						{
							continue;
						}
					}

					copy_arr(current_solution, current_neighbour);
					TS_generate_neighbour(current_neighbour, i, j);

					neighbour_cost = path_len(current_neighbour);

					if (neighbour_cost < near_best_cost)
					{
						copy_arr(current_neighbour, near_best_solution);
						near_best_cost = neighbour_cost;
						cur_move[0] = i;
						cur_move[1] = j;
					}
				}
			}
			if (near_best_cost < global_best_cost)
			{
				copy_arr(near_best_solution, global_best_solution);
				global_best_cost = near_best_cost;
				//std::cout << "IMPROVED:\t" << near_best_cost << "\n";
			}

			copy_arr(near_best_solution, current_solution);

			taboo_list[cur_move[1]][cur_move[0]] = size * size / 2; //reverse move to the list

			for (int i = 0; i < size; i++)
			{
				for (int j = 0; j < size; j++)
				{
					if (taboo_list[i][j] > 0)
					{
						taboo_list[i][j] -= 1;
					}
				}
			}


			elapsed = std::chrono::system_clock::now() - start;
		} while (elapsed.count() < run_limit);

		std::cout << "Time elapsed: " << elapsed.count() << "\nFinal path length: " << global_best_cost //<< "\n";
			<< "\nFinal path:\n" << str_path(global_best_solution) << "\n";

		save_path_to_file(global_best_solution);

		delete global_best_solution;
		delete near_best_solution;
		delete current_solution;
		delete current_neighbour;

		delete cur_move;

		for (int i = 0; i < size; i++)
		{
			delete taboo_list[i];
		}
		delete taboo_list;
	}

	//UI functions

	std::string str_cooling()
	{
		return "Cooling method: " + cooling_schedules[chosen_cooling_schedule] + "\nCooling coefficient: " + std::to_string(cooling_coefficient) + "\n";
	}

	void set_stop_conditions()
	{
		int time_con;
		std::cout << "Enter a new time constraint (in seconds): ";
		std::cin >> time_con;
		if (test_input_validity("ERROR - not an integer!"))
			run_limit = time_con;

		system("cls");
		std::cout << "Time constraint: " << run_limit << " seconds\n";
		system("pause");
	}

	void set_cooling_schedule()
	{
		std::string hints[3] = { "Formula: T'=T*coeff\nCoefficients must be in (0, 1), preferebly very close to 1",
			"Formula: T'=T/(1+coeff*T)\nCoefficients must be in (0, 1), preferebly very close to 0",
			"Formula: T'=T-coeff\nCoefficients must be in (0, T_start), preferebly small" };
		chosen_cooling_schedule = create_adaptive_sub_menu(str_cooling(), cooling_schedules, hints, 3, 0);

		system("cls");
		std::cout << str_cooling();
		system("pause");

	}

	void set_cooling_coefficient()
	{
		system("cls");
		double new_coeff;
		std::cout << str_cooling();
		std::cout << "Input the new cooling coefficient:\n";
		std::cin >> new_coeff;
		if (test_input_validity("ERROR - not a double!"))
			cooling_coefficient = new_coeff;

		system("cls");
		std::cout << str_cooling();
		system("pause");
	}

	void set_neighbourhood()
	{
		chosen_definition = create_sub_menu("Choose neighbourhood definition\n", neighbour_defs, "", 3, 0);

		system("cls");
		std::cout << "Current neighbourhood definition: " << neighbour_defs[chosen_definition] << "\n";
		system("pause");

	}

	void read_path_from_file()
	{
		if (size == 0)
		{
			std::cout << "No data loaded!\n";
			system("pause");
			return;
		}

		std::string filename;
		std::cout << "Enter filename:\n";
		std::cin >> filename;
		if (!test_input_validity("Unsupported filename!\n"))
			return;

		std::ifstream file(filename);
		if (!file.is_open())
		{
			std::cout << "File open error!\n";
			system("pause");
			return;
		}

		int s;
		file >> s;

		if (s != size)
		{
			std::cout << "Size mismatch error!\n";
			file.close();
			system("pause");
			return;
		}

		int* path = new int[size];

		for (int i = 0; i < size; i++)
		{
			file >> path[i];
		}
		file.close();

		std::cout << "Read path has a length of " << path_len(path) << "\n";

		delete path;
		system("pause");
	}

public:
	void run()
	{
		std::string title = "MAIN MENU\n";
		std::string credits = "Kuba Bigaj 2023\n";
		std::string options[10] = { "Load data", "Show current data","Set stop conditions", "Simulated annealing", "[SA] Set cooling schedule","[SA] Set cooling coefficient", "Taboo Search", "[TS] Set neighbourhood definition", "Read path from text file",  "Exit" };
		int chosen_option = 0;

		while (true)
		{
			chosen_option = create_sub_menu(title, options, credits, 10, chosen_option);

			switch (chosen_option)
			{
			case 0:
				load_data();
				break;
			case 1:
				show_data();
				break;
			case 2:
				set_stop_conditions();
				break;
			case 3:
				simulated_annealing();
				system("pause");
				break;
			case 4:
				set_cooling_schedule();
				break;
			case 5:
				set_cooling_coefficient();
				break;
			case 6:
				taboo_search();
				system("pause");
				break;
			case 7:
				set_neighbourhood();
				break;
			case 8:
				read_path_from_file();
				break;
			case 9:
				return;
			default:
				break;
			}
		}
	}

	void debug()
	{
		read_data_from_file("ftv55.atsp");

		run_limit = 120;
	}

	void run_tests()
	{
		read_data_from_file("ftv55.atsp");
		//170: 240 / cooling_coefficient = 0.999985;
		//55: 120 / cooling_coefficient = 0.999996;
		run_limit = 120;
		cooling_coefficient = 0.999996;
		for (int i = 0; i < 10; i++)
		{
			simulated_annealing();
		}
		/*for (int i = 0; i < 10; i++)
		{
			taboo_search();
		}*/
	}
};


int main(int argc, char *argv[])
{
	srand(time(NULL));
	App a;
	if (argc > 1)
	{
		if (strcmp(argv[1], "-t") == 0)
		{
			a.run_tests();
			return 0;
		}
	}
	//a.run();
	a.debug();
	return 0;
}