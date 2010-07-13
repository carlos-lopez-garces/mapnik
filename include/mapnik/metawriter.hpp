/*****************************************************************************
 *
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2010 Hermann Kraus
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 *****************************************************************************/


#ifndef METAWRITER_HPP
#define METAWRITER_HPP

// Mapnik
#include <mapnik/box2d.hpp>
#include <mapnik/feature.hpp>

// Boost
#include <boost/utility.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/optional.hpp>

// STL
#include <set>
#include <string>


namespace mapnik {

/** All properties to be output by a metawriter. */
typedef std::set<std::string> metawriter_properties;

/** Abstract baseclass for all metawriter classes. */
class metawriter
{
    public:
        metawriter(metawriter_properties dflt_properties) : dflt_properties_(dflt_properties) {}
        virtual ~metawriter() {};
        /** Output a rectangular area.
          * \param box Area (in pixel coordinates)
          * \param feature The feature being processed
          * \param prj_trans Projection transformation
          * \param t Cooridnate transformation
          * \param properties List of properties to output
          */
        virtual void add_box(box2d<double> box, Feature const &feature,
                             proj_transform const& prj_trans,
                             CoordTransform const &t,
                             metawriter_properties const& properties = metawriter_properties())=0;
        virtual void start() {};
        virtual void stop() {};
        static metawriter_properties parse_properties(boost::optional<std::string> str);
    protected:
        metawriter_properties dflt_properties_;
};

typedef boost::shared_ptr<metawriter> metawriter_ptr;
typedef std::pair<metawriter_ptr, metawriter_properties> metawriter_with_properties;

};

#endif