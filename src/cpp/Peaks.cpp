//
// Copyright 2015 Stephen Parker
//
// Licensed under Version 3 of the GPL or any later version
//

#include <algorithm>
#include <iostream>
#include <sstream>

#include <boost/chrono.hpp>

#include "Peaks.hpp"


bool operator< (const Peak& p1, const Peak& p2) {
    return (
        sort_strings_numerically(p1.reference, p2.reference) ||
        (
            p1.reference == p2.reference &&
            (
                ((p1.start < p2.start) || (p1.end < p2.end)) ||
                (p1.overlapping_hqaa < p2.overlapping_hqaa) ||
                sort_strings_numerically(p1.name, p2.name)
            )
        )
    );
}


bool operator== (const Peak& p1, const Peak& p2) {
    return (p1.reference == p2.reference) && (p1.start == p2.start) && (p1.end == p2.end) && (p1.overlapping_hqaa == p2.overlapping_hqaa) && (p1.name == p2.name);
}


std::ostream& operator<<(std::ostream& os, const Peak& peak) {
    os << peak.reference << '\t' << peak.start << '\t' << peak.end << '\t' << peak.name;
    return os;
}


std::istream& operator>>(std::istream& is, Peak& peak) {
    std::string peak_string;
    std::stringstream peak_stream;
    std::getline(is, peak_string);
    peak_stream.str(peak_string);
    peak_stream >> peak.reference >> peak.start >> peak.end >> peak.name;
    peak.overlapping_hqaa = 0;
    return is;
}


bool peak_overlapping_hqaa_descending_comparator(const Peak& p1, const Peak& p2) {
    return p1.overlapping_hqaa > p2.overlapping_hqaa;
}


void ReferencePeakCollection::add(Peak& peak) {
    peaks.push_back(peak);

    if (reference != peak.reference) {
        if (reference.empty()) {
            reference = peak.reference;
        } else {
            throw std::out_of_range("Peak reference does not match collection.");
        }
    }

    if (start == 0 || start > peak.start) {
        start = peak.start;
    }

    if (end == 0 || end < peak.end) {
        end = peak.end;
    }
}


bool ReferencePeakCollection::overlaps(const Feature& feature) {
    return !peaks.empty() && reference == feature.reference && ((start <= feature.end) || (feature.start <= end));
}


void PeakTree::add(Peak& peak) {
    tree[peak.reference].add(peak);
}


bool PeakTree::empty() {
    return tree.empty();
}


ReferencePeakCollection* PeakTree::get_reference_peaks(const std::string& reference_name){
    return &tree[reference_name];
}


void PeakTree::increment_overlapping_hqaa(const Feature& hqaa) {
    ReferencePeakCollection* rpc = get_reference_peaks(hqaa.reference);
    if (rpc->overlaps(hqaa)) {
        auto peak = std::upper_bound(rpc->peaks.begin(), rpc->peaks.end(), hqaa);
        auto end = rpc->peaks.end();

        for (; peak != end; ++peak) {
            if (peak->overlaps(hqaa)) {
                peak->overlapping_hqaa++;
            } else {
                break;
            }
        }
    }
}


std::vector<Peak> PeakTree::list_peaks() {
    std::vector<Peak> peaks;
    for (auto ref_peaks : tree) {
        for (auto peak: ref_peaks.second.peaks) {
            peaks.push_back(peak);
        }
    }
    std::sort(peaks.begin(), peaks.end());
    return peaks;
}


std::vector<Peak> PeakTree::list_peaks_by_overlapping_hqaa_descending() {
    std::vector<Peak> peaks;
    for (auto ref_peaks : tree) {
        for (auto peak: ref_peaks.second.peaks) {
            peaks.push_back(peak);
        }
    }
    std::sort(peaks.begin(), peaks.end(), peak_overlapping_hqaa_descending_comparator);
    return peaks;
}


void PeakTree::print_reference_peak_counts(std::ostream* os) {
    std::ostream out(os ? os->rdbuf() : std::cout.rdbuf());
    for (auto refpeaks : tree) {
         out << refpeaks.first << " peak count: " << refpeaks.second.peaks.size() << std::endl;
    }
}


size_t PeakTree::size() const {
    size_t size = 0;
    for (auto refpeaks : tree) {
        size += refpeaks.second.peaks.size();
    }
    return size;
}
