#pragma once
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>

class base_entry
{
public:

	base_entry()
	{
		elements.clear();
		raw_row_data.clear();
	}
	base_entry(std::string row)
	{
		raw_row_data = row;
		internal_get_elements();
	}
	~base_entry()
	{
		elements.clear();
		raw_row_data.clear();
	}

private:
	std::vector<std::string> internal_get_elements()
	{
		std::string t;
		int itr = 0;

		// Iterate row data and seperate elements out.
		for (auto i : raw_row_data)
		{
			// Search for ',' delimiter character or end of line.
			if (i == ',' || itr == (raw_row_data.size() - 1))
			{
				// Push built string as element and clear buf.
				elements.push_back(t);
				t.clear();
			}
			else
			{
				// Didn't find anything continue building string.
				t.push_back(i);
			}
			itr++;
		}
		return elements;
	}

public:
	std::vector<std::string> get_elements() { return elements; }

	std::string raw_row_data;
	std::vector<std::string> elements;
};

class sensor_entry : base_entry
{
public:
	sensor_entry()
	{
		id.clear();
		date.clear();
		time.clear();
	}
	sensor_entry(std::string row) : base_entry(row)
	{
		internal_get_id();
		internal_get_date();

		inherited_elements = elements;

	}
	~sensor_entry()
	{
		id.clear();
		date.clear();
		time.clear();
	}

private:
	void internal_get_id()
	{
		id = elements[0];
	}
	void internal_get_date()
	{
		// Format element to get date/time.
		auto datetime = elements[1];

		const auto split = datetime.find_first_of(" ");
		if (split != std::string::npos)
		{
			time = datetime.substr(split + 1, datetime.size());
			date = datetime.substr(0, split);
		}
	}

public:

	std::string get_date() { return date; }
	std::string get_time() { return time; }
	std::string get_id() { return id; }
	std::vector<std::string> get_elements() { return inherited_elements; }

protected:
	std::vector<std::string> inherited_elements;
	std::string id;
	std::string date;
	std::string time;
};

class temperature_sensor_entry : sensor_entry
{
public:
	temperature_sensor_entry()
	{
		temperature.clear();
		measure_unit.clear();
		signal_strength.clear();
		voltage.clear();
	}
	temperature_sensor_entry(std::string row) : sensor_entry(row)
	{
		internal_get_temperature();
		internal_get_measure_unit();
		internal_get_signal_strength();
		internal_get_voltage();

		to_json_entry();
	}
	~temperature_sensor_entry()
	{
		temperature.clear();
		measure_unit.clear();
		signal_strength.clear();
		voltage.clear();
	}

private:
	void internal_get_temperature()
	{
		temperature = inherited_elements[2];
	}
	void internal_get_measure_unit()
	{
		auto temp = inherited_elements[2];
		auto temp_with_units = inherited_elements[3];

		temp_with_units.erase(temp_with_units.begin(), temp_with_units.begin() + temp.size());
		measure_unit = temp_with_units;
	}
	void internal_get_signal_strength()
	{
		signal_strength = inherited_elements[4];
	}
	void internal_get_voltage()
	{
		voltage = inherited_elements[5];

		// Fix to prevent outputting "2." to JSON.
		if (voltage.back() == '.')
			voltage.pop_back();
	}

public:

	std::string get_temperature() { return temperature; }
	std::string get_measure_unit() { return measure_unit; }
	std::string get_signal_strength() { return signal_strength; }
	std::string get_voltage() { return voltage; }
	std::string get_date() { return date; }
	std::string get_time() { return time; }
	std::string get_id() { return id; }

	// Build json entry using extracted elements.
	std::string to_json_entry(bool is_last_entry = false)
	{
		std::string json;

		json.append("{\n");

		json.append("\"Date\": \"");
		json.append(date);
		json.append("\",\n");

		json.append("\"Time\": \"");
		json.append(time);
		json.append("\",\n");

		json.append("\"Temperature\": ");
		json.append(temperature);
		json.append(",\n");

		json.append("\"Unit\": \"");
		json.append(measure_unit);
		json.append("\",\n");

		json.append("\"Signal Strength\": ");
		json.append(signal_strength);
		json.append(",\n");

		json.append("\"Voltage\": ");
		json.append(voltage);

		if (!is_last_entry)
			json.append("\n},\n");
		else
			json.append("\n}\n");

		return json;
	}

private:
	std::string temperature;
	std::string measure_unit;
	std::string signal_strength;
	std::string voltage;
};


// Handles csv file using custom-class template for entries.
template <class T>
class csv_file
{
public:
	csv_file()
	{
		buf.clear();
		entries.clear();

		buf_size = 0;
	}

	csv_file(std::string file_path)
	{
		file.open(file_path, std::ios_base::in | std::ios_base::out);

		if (!file.is_open())
		{
			std::cout << "[-] Failed to open file! Path: " << file_path << std::endl;
			exit(-1);
		}

		buf << file.rdbuf();
		buf_size = sizeof(buf);

		file.close();
	}
	~csv_file()
	{
		if (file.is_open())
			file.close();

		buf.clear();
		entries.clear();

		buf_size = 0;
	}

	std::fstream file;

	std::stringstream buf;
	size_t buf_size;

	std::vector<T> entries;

	std::vector<T> get_entries()
	{
		std::string t;
		int rows = 0;

		for (auto i : buf.str())
		{
			if (i == '\n')
			{
				// End of row entry, pushback string into 'entries' vector.
				if (rows <= 0)
					t.clear();
				else
				{
					// First row is omitted as it only contains header data.
					entries.push_back(T(t));
					t.clear();
				}
				rows++;
			}
			else
			{
				// Add char to row string.
				t.push_back(i);
			}
		}		
		return entries;
	}

	// JSON grouping functions.
	std::string create_json_group(std::string groupname)
	{
		std::string json;

		json.append("{\n");
		json.append("\"");
		json.append(groupname);
		json.append("\": [\n");

		return json;
	}
	std::string close_json_group(bool is_last_entry = true)
	{
		std::string json;
		json.append("]\n");

		if (is_last_entry)
			json.append("}");
		else
			json.append("},");

		return json;
	}
};