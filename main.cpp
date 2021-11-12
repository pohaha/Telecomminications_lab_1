
#include <math.h>
#include <map>
#include <iostream>
#include <stdlib.h>
#include <time.h>

#define BIG_CITY 3
#define SMALL_CITY 0

#define STATISTIC_WIDTH 100
//variant # 7
float bandwidth = 20;                 //delta f = 20 MHz
float frequency = 1800;               //MHz
float reciever_noise_koefficient = 2; //Kn
float station_radiation_power = 10;   //10 Watt
int station_cover_radius = 1500;      //1500 Meters

//shared values
float temperature = 300;                   //300 K
float station_height = 30;                 //30 Meters
long double k = 1.38064852 * pow(10, -23); //boltzman constant

long double hata_propogation_function(float distance, int city_size_koefficient)
{
    float logarithmic_value = 0;
    logarithmic_value = 46.3 + 33.9 * log(frequency) - 13.82 * log(station_height) - (1.1 * log(frequency) - 0.7) * station_height + (1, 56 * log(frequency) - 0.8) + (44.9 - 6.55 * log(station_height)) * log(distance) + city_size_koefficient;
    return exp(logarithmic_value / 10);
}

float count_channel_capacity(float distance, int city_size_koefficient)
{
    float power_over_area_loss_koefficient = hata_propogation_function(distance, city_size_koefficient);
    float recieved_signal_power = station_radiation_power / power_over_area_loss_koefficient;
    float heat_noise_power = bandwidth * temperature * k * reciever_noise_koefficient;
    float SNR = recieved_signal_power / heat_noise_power;
    return (bandwidth * log2(1 + SNR));
}

float *generate_random_user_locations(int number_of_users)
{
    float *user_locations = new float[number_of_users];
    srand(time(NULL));
    for (int i = 0; i < number_of_users; i++)
    {
        user_locations[i] = rand() % station_cover_radius;
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
    std::map<int, float> math_wait_summ_speed;
    std::map<int, float> math_wait_average_speed;
    std::map<int, float> math_wait_minimal_speed;
    float single_measure_summ_speed[STATISTIC_WIDTH];
    float single_measure_average_speed[STATISTIC_WIDTH];
    float single_measure_minimal_speed[STATISTIC_WIDTH];

public:
    Planner(/* args */);
    virtual void count_single_measure_data(float *user_capacities, int num_of_users, int statistic_number) = 0;
    void show_data_as_graph();
    void fill_data(int num_of_users)
    {
        for (int i = 0; i < STATISTIC_WIDTH; i++)
        {
            float *user_locations = generate_random_user_locations(num_of_users);
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
        return local_summ / STATISTIC_WIDTH;
    }
    ~Planner();
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
        single_measure_average_speed[statistic_number] = 1 / single_measure_summ_speed[statistic_number];
        single_measure_summ_speed[statistic_number] = single_measure_average_speed[statistic_number] * num_of_users;
        single_measure_minimal_speed[statistic_number] = single_measure_average_speed[statistic_number];
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
};
int main()
{
}