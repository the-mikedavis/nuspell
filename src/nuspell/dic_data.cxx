/* Copyright 2016-2017 Dimitrij Mijoski
 *
 * This file is part of Nuspell.
 *
 * Nuspell is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Nuspell is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Nuspell.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "dic_data.hxx"

#include "locale_utils.hxx"
#include "string_utils.hxx"
#include <algorithm>
#include <iostream>
#include <limits>
#include <sstream>

//#include <boost/locale.hpp>

namespace hunspell {

using namespace std;

auto Dic_Data::parse(istream& in, const Aff_Data& aff) -> bool
{
	size_t line_number = 1;
	size_t approximate_size;
	istringstream ss;
	string line;

	// locale must be without thousands separator.
	// boost::locale::generator locale_generator;
	// auto loc = locale_generator("en_US.us-ascii");
	auto loc = locale::classic();
	in.imbue(loc);
	ss.imbue(loc);
	if (!getline(in, line)) {
		return false;
	}
	if (aff.encoding.is_utf8() && !validate_utf8(line)) {
		cerr << "Invalid utf in dic file" << endl;
	}
	ss.str(line);
	if (ss >> approximate_size) {
		words.reserve(approximate_size);
	}
	else {
		return false;
	}

	string word;
	string morph;
	vector<string> morphs;
	u16string flags;

	while (getline(in, line)) {
		line_number++;
		ss.str(line);
		ss.clear();
		word.clear();
		morph.clear();
		flags.clear();
		morphs.clear();

		if (aff.encoding.is_utf8() && !validate_utf8(line)) {
			cerr << "Invalid utf in dic file" << endl;
		}
		if (line.find('/') != line.npos) {
			// slash found, word untill slash
			getline(ss, word, '/');
			if (aff.flag_aliases.empty()) {
				flags = aff.decode_flags(ss);
			}
			else {
				size_t flag_alias_idx;
				ss >> flag_alias_idx;
				if (ss.fail() ||
				    flag_alias_idx > aff.flag_aliases.size()) {
					continue;
				}
				flags = aff.flag_aliases[flag_alias_idx - 1];
			}
		}
		else if (line.find('\t') != line.npos) {
			// Tab found, word until tab. No flags.
			// After tab follow morphological fields
			getline(ss, word, '\t');
		}
		else {
			// No slash or tab, treat word until first space.
			ss >> word;
			if (ss.fail()) {
				// probably all whitespace
				continue;
			}
		}
		if (word.empty()) {
			continue;
		}
		parse_morhological_fields(ss, morphs);
		words[word] += flags;
		if (morphs.size()) {
			auto& vec = morph_data[word];
			vec.insert(vec.end(), morphs.begin(), morphs.end());
		}
	}
	return in.eof(); // success if we reached eof
}
}
