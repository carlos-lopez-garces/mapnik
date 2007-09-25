/*****************************************************************************
 * 
 * This file is part of Mapnik (c++ mapping toolkit)
 *
 * Copyright (C) 2007 Artem Pavlenko
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

#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/parserInternals.h>
 
#include <boost/utility.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/filesystem/operations.hpp>

#include <mapnik/config_error.hpp>

#include <iostream>

using boost::property_tree::ptree;
using namespace std;

namespace mapnik 
{
    class libxml2_loader : boost::noncopyable
    {
        public:
            libxml2_loader() :
                ctx_( 0 )
            {
                LIBXML_TEST_VERSION;
            }

            ~libxml2_loader()
            {
                if (ctx_ && ctx_->myDoc)
                {
                    xmlFreeDoc( ctx_->myDoc );
                }
                if (ctx_)
                {
                    xmlFreeParserCtxt(ctx_);    
                }    
            }

            void load( const std::string & filename, ptree & pt )
            {
                boost::filesystem::path path(filename);
                if ( ! boost::filesystem::exists( path ) ) {
                    throw config_error(string("Could not load map file '") +
                            filename + "': File does not exist");
                }
                ctx_ = xmlCreateFileParserCtxt( filename.c_str() );

                if ( ! ctx_ ) 
                {
                    throw std::runtime_error("Failed to create parser context.");
                }

                ctx_->replaceEntities = true;
                ctx_->keepBlanks = false;
                xmlCtxtUseOptions( ctx_, XML_PARSE_NOERROR | XML_PARSE_NOENT | XML_PARSE_NOBLANKS |
                                         XML_PARSE_DTDLOAD);
                xmlParseDocument( ctx_ );

                if ( ! ctx_->wellFormed ) 
                {
                    xmlError * error = xmlCtxtGetLastError( ctx_ );
                    std::ostringstream os;
                    os << "XML document not well formed";
                    if (error)
                    {
                        os << ": " << std::endl << error->message;
                        // remove CR 
                        std::string msg = os.str().substr(0, os.str().size() - 1);
                        config_error ex( msg );

                        os.str("");
                        os << "in file '" << error->file << "' at line "
                            << error->line;

                        ex.append_context( os.str() );

                        throw ex;
                    }
                }

                /*
                   if ( ! ctx->valid ) 
                   {
                   std::clog << "### ERROR: Failed to validate DTD."
                   << std::endl;
                   }
                 */

                xmlNode * root( 0 );
                root = xmlDocGetRootElement( ctx_->myDoc );
                if ( ! root ) {
                    throw config_error("XML document is empty.");
                }

                populate_tree( root, pt );

            }

        private:
            void append_attributes( xmlAttr * attributes, ptree & pt)
            {
                if (attributes)
                {
                    ptree::iterator it = pt.push_back( ptree::value_type( "<xmlattr>", ptree() ));
                    ptree & attr_list = it->second;
                    xmlAttr * cur_attr = attributes;
                    for (; cur_attr; cur_attr = cur_attr->next )
                    {
                        ptree::iterator it = attr_list.push_back(
                                ptree::value_type( (char*)cur_attr->name, ptree() ));
                        it->second.put_own( (char*) cur_attr->children->content );
                    }
                }
            }

            void populate_tree( xmlNode * node, ptree & pt )
            {
                xmlNode * cur_node = node;

                for (; cur_node; cur_node = cur_node->next )
                {
                    switch (cur_node->type) 
                    {
                        case XML_ELEMENT_NODE:
                            {
                                ptree::iterator it = pt.push_back( ptree::value_type(
                                        (char*)cur_node->name, ptree() ));
                                append_attributes( cur_node->properties, it->second);
                                populate_tree( cur_node->children, it->second );
                            }
                            break;
                        case XML_TEXT_NODE:
                            pt.put_own( (char*) cur_node->content );
                            break;
                        case XML_COMMENT_NODE:
                            {
                                ptree::iterator it = pt.push_back(
                                        ptree::value_type( "<xmlcomment>", ptree() ));
                                it->second.put_own( (char*) cur_node->content );
                            }
                            break;
                        default:
                            break;

                    }
                }
            }

            xmlParserCtxtPtr ctx_;
    };

    void read_xml2( std::string const & filename, boost::property_tree::ptree & pt)
    {
        libxml2_loader loader;
        loader.load( filename, pt );
    }

} // end of namespace mapnik