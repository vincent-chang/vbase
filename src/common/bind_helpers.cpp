#include "duckdb/common/bind_helpers.hpp"
#include "duckdb/common/common.hpp"
#include "duckdb/common/types.hpp"
#include "duckdb/common/exception.hpp"
#include "duckdb/common/types/value.hpp"
#include "duckdb/common/unordered_map.hpp"

namespace duckdb {

Value ConvertVectorToValue(vector<Value> set) {
	if (set.empty()) {
		return Value::EMPTYLIST(LogicalType::BOOLEAN);
	}
	return Value::LIST(move(set));
}

vector<bool> ParseColumnList(const vector<Value> &set, vector<string> &names, const string &loption) {
	vector<bool> result;

	if (set.empty()) {
		throw BinderException("\"%s\" expects a column list or * as parameter", loption);
	}
	// list of options: parse the list
	unordered_map<string, bool> option_map;
	for (idx_t i = 0; i < set.size(); i++) {
		option_map[set[i].ToString()] = false;
	}
	result.resize(names.size(), false);
	for (idx_t i = 0; i < names.size(); i++) {
		auto entry = option_map.find(names[i]);
		if (entry != option_map.end()) {
			result[i] = true;
			entry->second = true;
		}
	}
	for (auto &entry : option_map) {
		if (!entry.second) {
			throw BinderException("\"%s\" expected to find %s, but it was not found in the table", loption,
			                      entry.first.c_str());
		}
	}
	return result;
}

vector<bool> ParseColumnList(const Value &value, vector<string> &names, const string &loption) {
	vector<bool> result;

	// Only accept a list of arguments
	if (value.type().id() != LogicalTypeId::LIST) {
		// Support a single argument if it's '*'
		if (value.type().id() == LogicalTypeId::VARCHAR && value.GetValue<string>() == "*") {
			result.resize(names.size(), true);
			return result;
		}
		throw BinderException("\"%s\" expects a column list or * as parameter", loption);
	}
	auto &children = ListValue::GetChildren(value);
	// accept '*' as single argument
	if (children.size() == 1 && children[0].type().id() == LogicalTypeId::VARCHAR &&
	    children[0].GetValue<string>() == "*") {
		result.resize(names.size(), true);
		return result;
	}
	return ParseColumnList(children, names, loption);
}

} // namespace duckdb
