
#include <math.h>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <random>
#include <vector>
#include "includes/matplotlib-cpp.h"
#include <unistd.h>
#define BIG_CITY 3
#define SMALL_CITY 0

#define STATISTIC_WIDTH 100
// variant # 7
float bandwidth = 20000000;           // delta f = 20 MHz
float frequency = 1800;               // MHz
float reciever_noise_koefficient = 2; // n  = 2Kn
float station_radiation_power = 160;  // Watt
int station_cover_radius = 1500;      // 1500 Meters

// shared values
float temperature = 300;                   // 300 K
float station_height = 30;                 // 30 Meters
float cellphone_height = 1;                // Meter
long double k = 1.38064852 * pow(10, -23); // boltzman constant

long double hata_propogation_function(float distance, int city_size_koefficient)
{
    float logarithmic_value = 0;
    logarithmic_value = 46.3 + 33.9 * log10(frequency) - 13.82 * log10(station_height) - (1.1 * log10(frequency) - 0.7) * cellphone_height + (1.56 * log10(frequency) - 0.8) + (44.9 - 6.55 * log10(cellphone_height)) * log10(distance * 0.001) + float(city_size_koefficient);
    return pow(10, logarithmic_value / 10.0);
}

float count_channel_capacity(float distance, int city_size_koefficient)
{
    double power_over_area_loss_koefficient = hata_propogation_function(distance, city_size_koefficient);
    double recieved_signal_power = station_radiation_power / power_over_area_loss_koefficient;
    double heat_noise_power = bandwidth * temperature * k * reciever_noise_koefficient;
    double SNR = recieved_signal_power / heat_noise_power;
    return (bandwidth * log2(1 + SNR));
}

// random values uniform distribution
std::random_device rd;  // Will be used to obtain a seed for the random number engine
std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
std::uniform_int_distribution<> distrib(1, station_cover_radius);

int *generate_random_user_locations(int number_of_users)
{
    int *user_locations = new int[number_of_users];
    for (int i = 0; i < number_of_users; i++)
    {
        user_locations[i] = distrib(gen);
    }
    return user_locations;
}

float find_minimum(float *array_to_serarch, int number_of_elements)
{
    float minimal_value = array_to_serarch[0];
    for (int i = 1; i < number_of_elements; i++)
    {
        if (array_to_serarch[i] < minimal_value)
        {
            minimal_value = array_to_serarch[i];
        }
    }
    return minimal_value;
}

float find_maximum(float *array_to_search, int number_of_elements)
{
    float max_value = array_to_search[0];
    for (int i = 1; i < number_of_elements; i++)
    {
        if (array_to_search[i] > max_value)
        {
            max_value = array_to_search[i];
        }
    }
    return max_value;
}

enum class Data_Type
{
    summ_speed,
    average_speed,
    minimal_speed
};
class Planner
{
protected:
    std::string name = "none";
    std::map<int, float> math_wait_summ_speed;
    std::map<int, float> math_wait_average_speed;
    std::map<int, float> math_wait_minimal_speed;
    float single_measure_summ_speed[STATISTIC_WIDTH];
    float single_measure_average_speed[STATISTIC_WIDTH];
    float single_measure_minimal_speed[STATISTIC_WIDTH];

public:
    Planner(/* args */) = default;

    Planner(const std::string &name) : name(name)
    {
    }

    virtual void count_single_measure_data(float *user_capacities, int num_of_users, int statistic_number) = 0;

    void show_data(std::vector<int> user_numbers, long plot_number)
    {
        namespace plt = matplotlibcpp;
        std::vector<float> y_axis_summ_speed;
        std::vector<float> y_axis_average_speed;
        std::vector<float> y_axis_minimal_speed;
        for (int key = 0; key < user_numbers.size(); key++)
        {
            y_axis_summ_speed.push_back(math_wait_summ_speed[user_numbers[key]]);
            y_axis_average_speed.push_back(math_wait_average_speed[user_numbers[key]]);
            y_axis_minimal_speed.push_back(math_wait_minimal_speed[user_numbers[key]]);
        }
        plt::subplot(3, 1, plot_number);
        plt::named_plot("minimal speed", user_numbers, y_axis_minimal_speed);
        plt::named_plot("summ speed", user_numbers, y_axis_summ_speed);
        plt::named_plot("average speed", user_numbers, y_axis_average_speed);
        plt::title(name);
        plt::xlabel("number of users");
        plt::ylabel("math wait of 100 samples");
        plt::legend();
    }
    void fill_data(int num_of_users)
    {
        for (int i = 0; i < STATISTIC_WIDTH; i++)
        {
            int *user_locations = generate_random_user_locations(num_of_users);
            float *channel_capacity_per_user = new float[num_of_users];
            for (int j = 0; j < num_of_users; j++)
            {
                channel_capacity_per_user[j] = count_channel_capacity(user_locations[j], BIG_CITY);
            }
            count_single_measure_data(channel_capacity_per_user, num_of_users, i);
            delete[] channel_capacity_per_user;
            delete[] user_locations;
        }
        math_wait_average_speed[num_of_users] = count_statistic_data(Data_Type::average_speed);
        math_wait_summ_speed[num_of_users] = count_statistic_data(Data_Type::summ_speed);
        math_wait_minimal_speed[num_of_users] = count_statistic_data(Data_Type::minimal_speed);
    }
    float count_statistic_data(Data_Type type_to_fill)
    {
        float *currently_counted_data = nullptr;
        float local_summ = 0;
        switch (type_to_fill)
        {
        case Data_Type::summ_speed:
            currently_counted_data = single_measure_summ_speed;
            break;
        case Data_Type::average_speed:
            currently_counted_data = single_measure_average_speed;
            break;
        case Data_Type::minimal_speed:
            currently_counted_data = single_measure_minimal_speed;
            break;

        default:
            std::cout << "errors_occured in counting statistics" << std::endl;
            break;
        }
        for (int i = 0; i < STATISTIC_WIDTH; i++)
        {
            local_summ += currently_counted_data[i];
        }
        return local_summ / float(STATISTIC_WIDTH);
    }
    ~Planner() = default;
};

class PRS : public Planner
{
    void count_single_measure_data(float *user_capacities, int num_of_users, int statistic_number) override
    {
        single_measure_summ_speed[statistic_number] = 0;
        single_measure_average_speed[statistic_number] = 0;
        single_measure_minimal_speed[statistic_number] = 0;
        for (int i = 0; i < num_of_users; i++)
        {
            single_measure_summ_speed[statistic_number] += (1.0 / user_capacities[i]);
        }
        single_measure_average_speed[statistic_number] = 1.0 / single_measure_summ_speed[statistic_number];
        single_measure_summ_speed[statistic_number] = single_measure_average_speed[statistic_number] * float(num_of_users);
        single_measure_minimal_speed[statistic_number] = single_measure_average_speed[statistic_number];
    }

public:
    PRS() : Planner("Equal speed planner")
    {
    }
};
class PSS : public Planner
{
    void count_single_measure_data(float *user_capacities, int num_of_users, int statistic_number) override
    {
        single_measure_summ_speed[statistic_number] = find_maximum(user_capacities, num_of_users);
        single_measure_average_speed[statistic_number] = single_measure_summ_speed[statistic_number] / num_of_users;
        single_measure_minimal_speed[statistic_number] = 0;
    }

public:
    PSS() : Planner("Max summ speed planner")
    {
    }
};
class PRD : public Planner
{
    void count_single_measure_data(float *user_capacities, int num_of_users, int statistic_number) override
    {
        single_measure_summ_speed[statistic_number] = 0;
        single_measure_average_speed[statistic_number] = 0;
        single_measure_minimal_speed[statistic_number] = 0;
        float user_speeds[num_of_users];
        for (int i = 0; i < num_of_users; i++)
        {
            user_speeds[i] = user_capacities[i] / num_of_users;
            single_measure_summ_speed[statistic_number] += user_speeds[i];
        }
        single_measure_average_speed[statistic_number] = single_measure_summ_speed[statistic_number] / num_of_users;
        single_measure_minimal_speed[statistic_number] = find_minimum(user_speeds, num_of_users);
    }

public:
    PRD() : Planner("Equal share planner")
    {
    }
};

int main()
{
    // PRS prs_planner;
    // PRD prd_planner;
    // PSS pss_planner;
    // std::vector<int> user_numbers = {1, 100, 200, 300, 400, 500};
    // for (int num_id = 0; num_id < 6; num_id++)
    // {
    //     prs_planner.fill_data(user_numbers[num_id]);
    //     pss_planner.fill_data(user_numbers[num_id]);
    //     prd_planner.fill_data(user_numbers[num_id]);
    // }
    // prs_planner.show_data(user_numbers, 1);
    // pss_planner.show_data(user_numbers, 2);
    // prd_planner.show_data(user_numbers, 3);
    // matplotlibcpp::show();
    std::cout << count_channel_capacity(100, BIG_CITY) << std::endl;
}