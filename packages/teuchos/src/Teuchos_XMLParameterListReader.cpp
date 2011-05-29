// @HEADER
// ***********************************************************************
//
//                    Teuchos: Common Tools Package
//                 Copyright (2004) Sandia Corporation
//
// Under terms of Contract DE-AC04-94AL85000, there is a non-exclusive
// license for use of this work by or on behalf of the U.S. Government.
//
// This library is free software; you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as
// published by the Free Software Foundation; either version 2.1 of the
// License, or (at your option) any later version.
//
// This library is distributed in the hope that it will be useful, but
// WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
// USA
// Questions? Contact Michael A. Heroux (maherou@sandia.gov)
//
// ***********************************************************************
// @HEADER

#include "Teuchos_XMLParameterListReader.hpp"
#include "Teuchos_TestForException.hpp"
#include "Teuchos_StrUtils.hpp"

using namespace Teuchos;

XMLParameterListReader::XMLParameterListReader()
{;}

ParameterList XMLParameterListReader::toParameterList(const XMLObject& xml) const
{
  TEST_FOR_EXCEPTION(xml.getTag() != "ParameterList", std::runtime_error,
                     "XMLParameterListReader expected tag ParameterList, found "
                     << xml.getTag());

  ParameterList rtn;
  
  if (xml.hasAttribute("name"))
    {
      rtn.setName(xml.getAttribute("name"));
    }

  for (int i=0; i<xml.numChildren(); i++)
    {
      XMLObject child = xml.getChild(i);

      TEST_FOR_EXCEPTION( (child.getTag() != "ParameterList" 
                           && child.getTag() != "Parameter"), 
                         std::runtime_error,
                         "XMLParameterListReader expected tag "
                         "ParameterList or Parameter, found "
                         << child.getTag());

      if (child.getTag()=="ParameterList")
        {
          const std::string& name = child.getRequired("name");

          ParameterList sublist = toParameterList(child);
          sublist.setName(name);

          rtn.set(name, sublist);
        }
      else
        {
          const std::string& name = child.getRequired("name");
          const std::string& type = child.getRequired("type");
          
          bool isDefault = false;
          bool isUsed = false;
          if (child.hasAttribute("isDefault")) 
          {
            isDefault = child.getRequiredBool("isDefault");
          }
          if (child.hasAttribute("isUsed")) 
          {
            isUsed = child.getRequiredBool("isUsed");
          }

          // setValue assigns isUsed to false
          // getValue assigns isUsed to true

          ParameterEntry entry;
          if (type=="double" || type=="float")
            {
              entry.setValue<double>(child.getRequiredDouble("value"), 
                                     isDefault);
              if (isUsed) {
                double tmp = entry.getValue<double>(&tmp);
              }
            }
          else if (type=="short")
            {
              entry.setValue<short>(child.getRequiredInt("value"), 
                                  isDefault);
              if (isUsed) {
                short tmp = entry.getValue<short>(&tmp);
              }
            }
          else if (type=="int")
            {
              entry.setValue<int>(child.getRequiredInt("value"), 
                                  isDefault);
              if (isUsed) {
                int tmp = entry.getValue<int>(&tmp);
              }
            }
          else if (type=="bool")
            {
              entry.setValue<bool>(child.getRequiredBool("value"), 
                                   isDefault);
              if (isUsed) {
                bool tmp = entry.getValue<bool>(&tmp);
              }
            }
          else if (type=="string")
            {
              entry.setValue<std::string>(child.getRequired("value"), 
                                     isDefault);
              if (isUsed) {
                std::string tmp = entry.getValue<std::string>(&tmp);
              }
            }
					else if (type=="Array int")
						{
							entry.setValue<Array<int> >(Teuchos::fromStringToArray<int>(child.getRequired("value")),
																			isDefault);
							if (isUsed) {
								Array<int> tmp = entry.getValue<Array<int> >(&tmp);
							}
						}
					else if (type=="Array short")
						{
							entry.setValue<Array<short> >(Teuchos::fromStringToArray<short>(child.getRequired("value")),
																			isDefault);
							if (isUsed) {
								Array<short> tmp = entry.getValue<Array<short> >(&tmp);
							}
						}
					else if (type=="Array float")
						{
							entry.setValue<Array<float> >(Teuchos::fromStringToArray<float>(child.getRequired("value")),
																			isDefault);
							if (isUsed) {
								Array<float> tmp = entry.getValue<Array<float> >(&tmp);
							}
						}
					else if (type=="Array double")
						{
							entry.setValue<Array<double> >(Teuchos::fromStringToArray<double>(child.getRequired("value")),
																			isDefault);
							if (isUsed) {
								Array<double> tmp = entry.getValue<Array<double> >(&tmp);
							}
						}
					else if (type=="Array string")
						{
							entry.setValue<Array<std::string> >(Teuchos::fromStringToArray<std::string>(child.getRequired("value")),
																			isDefault);
							if (isUsed) {
								Array<std::string> tmp = entry.getValue<Array<std::string> >(&tmp);
							}
						}
          else 
            {
              entry.setValue<std::string>(child.getRequired("value"), 
                                   isDefault);
              if (isUsed) {
                std::string tmp = entry.getValue<std::string>(&tmp);
              }
            }
          rtn.setEntry(name, entry);
        }
                         
    }

  return rtn;
                     
}
