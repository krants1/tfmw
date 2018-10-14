#ifndef __T_INI_FILES_H__
#define __T_INI_FILES_H__

#include <utility>
#include <string>
#include <set>
#include <fstream>
#include <list>
#include <vector>
#include <unordered_map>
#include "boost/lexical_cast.hpp"
#include "log.h"

namespace T {
	class IniFile : private std::unordered_map<std::string, std::string> {
	private:
		std::string fileName;
		std::string groupName;
		size_t groupLastLineNum = SIZE_MAX;
		std::unordered_map<std::string, int> keyLineNums;
		std::vector<std::string> lines;
		void clearAll() {
			unordered_map::clear();
			groupLastLineNum = SIZE_MAX;
			lines.clear();
			keyLineNums.clear();
		}
	protected:
		void load() {
			this->clearAll();
			std::ifstream file(fileName);
			std::string s;
			while (getline(file, s, '\n')) {
				lines.push_back(s);
			}
			this->parse();
		}
		bool containKey(const std::string &key) {
			return (this->count(key) > 0);
			//return !(this->find(key) == this->end());
		}
		void parse() {
			std::function<size_t(size_t, size_t)> stmax = [](size_t v1, size_t v2) {
				if (v1 == SIZE_MAX)
					v1 = 0;
				if (v2 == SIZE_MAX)
					v2 = 0;
				return (v1 > v2) ? v1 : v2;
			};

			std::string _groupName;
			int i = -1;
			for (std::string s : lines) {
				i++;
				s = trim(s);
				if (!s.empty() && (s.front() != ';') && (s.front() != '#')) {
					size_t p = s.find_first_of('=');
					if ((s.size() >= 3) && (s.front() == '[') && (s.back() == ']')) {
						_groupName = s.substr(1, s.size() - 2);
						if (_groupName == this->groupName)
							groupLastLineNum = stmax(groupLastLineNum, (size_t) i);
					} else if ((_groupName == this->groupName) && (p != std::string::npos)) {
						std::string n = trim(s.substr(0, p));
						if (!containKey(n)) {
							std::string v = trim(s.substr(p + 1));
							(*this)[n] = v;
							keyLineNums[n] = i;
						}
						groupLastLineNum = stmax(groupLastLineNum, (size_t) i);
					}
				}
			}
		}
		static std::string trim(std::string s) {
			while (!s.empty() && (s.back() == '\r' || s.back() == '\t' || s.back() == ' ')) s.pop_back();
			while (!s.empty() && (s.front() == '\r' || s.front() == '\t' || s.front() == ' ')) s = s.substr(1);
			return s;
		}
	public:
		explicit IniFile(std::string fileName = "", std::string groupName = "General") {
			if (fileName.empty()) {
				fileName = T::PathHelper::getExePath();
				T::PathHelper::replaceExt(fileName, "ini");
			}
			this->fileName = fileName;
			this->groupName = std::move(groupName);
			this->load();
		}
		std::string const getValue(const std::string &name, std::string defaultValue = "") {
			if (!containKey(name))
				return defaultValue;
			else
				return (*this)[name];
		}
		void save() {
			std::ofstream fileo(fileName, std::ofstream::out | std::ofstream::trunc);
			for (std::string &s : lines) {
				if (!s.empty() && s.back() != char(13)) s += char(13);
				fileo << s << char(10);
			}
		}
		void setValue(std::string name, const std::string &value) {
			name = trim(name);
			if (!containKey(name)) {
				std::vector<std::string>::iterator it;
				if (groupLastLineNum == SIZE_MAX) {
					lines.insert(lines.end(), "[" + this->groupName + "]");
					groupLastLineNum = lines.size() - 1;
				}
				it = lines.begin() + groupLastLineNum + 1;
				lines.insert(it, name + " = " + value);

				(*this)[name] = value;
				keyLineNums[name] = (int) groupLastLineNum + 1;

				groupLastLineNum++;
			} else {
				(*this)[name] = value;
				int i = keyLineNums[name];

				lines[i] = name + " = " + value;
			}
		}
		bool hasValue(std::string name) {
			name = trim(name);
			return containKey(name);
		}
	};

	template<typename T>
	class Param {
	private:
		void checkAllowTypes() {
			static const std::set<std::string> allowTypes({
					                                              typeid(int).name(),
					                                              typeid(std::string).name(),
					                                              typeid(bool).name(),
					                                              typeid(float).name(),
					                                              typeid(double).name(),
					                                              typeid(long).name()
			                                              });

			if (allowTypes.find(typeid(T).name()) == allowTypes.end()) {
				std::string s = "Unknow Field Type: ";
				s.append(typeid(T).name());
				throw std::runtime_error(s);
			}
		};
	protected:
		IniFile &owner;
		std::string paramName;
		T paramValue;
		T defaultValue;
		std::string defaultValueStr;
		T fromString(std::string value) {
			return boost::lexical_cast<T>(value);
		};
		std::string toString(T value) {
			//return templateTypeToString(value);
			return boost::lexical_cast<std::string>(value);
		};
	public:
		operator T() {
			return paramValue;
		}
		void operator=(T data) {
			paramValue = data;
			owner.setValue(paramName, toString(paramValue));
		}
		Param(IniFile &owner, const std::string &paramName, T defaultValue = T()) :
				owner(owner), paramName(paramName), defaultValue(defaultValue) {
			this->checkAllowTypes();

			defaultValueStr = toString(defaultValue);
			paramValue = fromString(owner.getValue(paramName, defaultValueStr));
			if (!owner.hasValue(paramName))
				owner.setValue(paramName, toString(paramValue));
		}
	};
}
#endif //__T_INI_FILES_H__
