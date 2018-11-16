#pragma once

#include <fstream>
#include <vector>
#include <unordered_map>
#include "boost/lexical_cast.hpp"
#include "paths.h"

namespace T {
	class IniFile : private std::unordered_map<std::string, std::string> {
	public:
		explicit IniFile(std::string fileName = "", std::string groupName = "General") {
			if (fileName.empty()) {
				fileName = T::PathHelper::getExePath();
				T::PathHelper::replaceExt(fileName, "ini");
			}
			fileName_ = fileName;
			groupName_ = std::move(groupName);
			load();
		}
		std::string const getValue(const std::string& name, std::string defaultValue = "") {
			if (!containKey(name))
				return defaultValue;
			else
				return (*this)[name];
		}
		void save() {
			std::ofstream fileo(fileName_, std::ofstream::out | std::ofstream::trunc);
			for (std::string& s : lines_) {
				if (!s.empty() && s.back() != char(13)) s += char(13);
				fileo << s << char(10);
			}
		}
		void setValue(std::string name, const std::string& value) {
			name = trim(name);
			if (!containKey(name)) {
				std::vector<std::string>::iterator it;
				if (groupLastLineNum_ == SIZE_MAX) {
					lines_.insert(lines_.end(), "[" + groupName_ + "]");
					groupLastLineNum_ = lines_.size() - 1;
				}
				it = lines_.begin() + groupLastLineNum_ + 1;
				lines_.insert(it, name + " = " + value);

				(*this)[name] = value;
				keyLineNums_[name] = (int) groupLastLineNum_ + 1;

				groupLastLineNum_++;
			} else {
				(*this)[name] = value;
				int i = keyLineNums_[name];

				lines_[i] = name + " = " + value;
			}
		}
		bool hasValue(std::string name) {
			name = trim(name);
			return containKey(name);
		}
	protected:
		void load() {
			clearAll();
			std::ifstream file(fileName_);
			std::string s;
			while (getline(file, s, '\n'))
				lines_.push_back(s);
			parse();
		}
		bool containKey(const std::string& key) {
			return (count(key) > 0);
		}
		void parse() {
			std::function<size_t(size_t, size_t)> stmax = [](size_t v1, size_t v2) {
				if (v1 == SIZE_MAX)
					v1 = 0;
				if (v2 == SIZE_MAX)
					v2 = 0;
				return (v1 > v2) ? v1 : v2;
			};

			std::string groupName;
			int i = -1;
			for (std::string s : lines_) {
				i++;
				s = trim(s);
				if (!s.empty() && (s.front() != ';') && (s.front() != '#')) {
					size_t p = s.find_first_of('=');
					if ((groupName == groupName_) && (p != std::string::npos)) {
						std::string n = trim(s.substr(0, p));
						if (!containKey(n)) {
							std::string v = trim(s.substr(p + 1));
							(*this)[n] = v;
							keyLineNums_[n] = i;
						}
						groupLastLineNum_ = stmax(groupLastLineNum_, (size_t) i);
					} else if ((s.size() >= 3) && (s.front() == '[') && (s.back() == ']')) {
						groupName = s.substr(1, s.size() - 2);
						if (groupName == groupName_)
							groupLastLineNum_ = stmax(groupLastLineNum_, (size_t) i);
					}
				}
			}
		}
		static std::string trim(std::string s) {
			while (!s.empty() && (s.back() == '\r' || s.back() == '\t' || s.back() == ' ')) s.pop_back();
			while (!s.empty() && (s.front() == '\r' || s.front() == '\t' || s.front() == ' ')) s = s.substr(1);
			return s;
		}
		void clearAll() {
			unordered_map::clear();
			groupLastLineNum_ = SIZE_MAX;
			lines_.clear();
			keyLineNums_.clear();
		}
	private:
		std::string fileName_;
		std::string groupName_;
		size_t groupLastLineNum_ = SIZE_MAX;
		std::unordered_map<std::string, int> keyLineNums_;
		std::vector<std::string> lines_;
	};

	template<typename T>
	class Param {
	public:
		operator T() {
			return paramValue_;
		}
		T& operator=(const T & data) {
			paramValue_ = data;
			owner_.setValue(paramName_, toString(paramValue_));
			return paramValue_;
		}
		Param(IniFile& owner, std::string paramName, T defaultValue = T()) :
			owner_(owner), paramName_(std::move(paramName)) {

			paramValue_ = fromString(owner_.getValue(paramName_, toString(defaultValue)));
			if (!owner_.hasValue(paramName_)) {
				owner_.setValue(paramName_, toString(paramValue_));
			}
		}
	protected:
		T fromString(std::string value) {
			return boost::lexical_cast<T>(value);
		};
		std::string toString(T value) {
			return boost::lexical_cast<std::string>(value);
		};
	private:
		IniFile & owner_;
		std::string paramName_;
		T paramValue_;
	};
}
