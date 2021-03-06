/*=========================================================================

  Program:   ParaView
  Module:    vtkSMProxyProperty.h

  Copyright (c) Kitware, Inc.
  All rights reserved.
  See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.

     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notice for more information.

=========================================================================*/
// .NAME vtkSMProxyProperty - property representing pointer(s) to vtkObject(s)
// .SECTION Description
// vtkSMProxyProperty is a concrete sub-class of vtkSMProperty representing
// pointer(s) to vtkObject(s) (through vtkSMProxy).
//
// vtkSMProperty::UpdateDomains() is called by vtkSMProperty itself whenever
// its unchecked values are modified. In case of proxy-properties, the dependent
// domains typically tend to depend on the data information provided by the
// source-proxies added to the property. Thus, to ensure that the domains get
// updated if the data information changes, vtkSMProxyProperty ensures that
// vtkSMProperty::UpdateDomains() is called whenever any of the added proxies
// fires the vtkCommand::UpdateDataEvent (which is fired whenever the pipeline
// us updated through the ServerManager indicating that the data-information
// last used may have been invalidated).
//
// Besides the standard set of attributes, the following XML attributes are
// supported:
// \li command : identifies the method to call on the VTK object e.g.
// AddRepresentation.
// \li clean_command : if present, called once before invoking the method
// specified by \c command every time the property value is pushed e.g.
// RemoveAllRepresentations. If property
// can take multiple values then the \c command is called for for each of the
// values after the clean command for every push.
// \li remove_command : an alternative to clean_command where instead of
// resetting and adding all the values for every push, this simply calls the
// specified method to remove the vtk-objects no longer referred to e.g.
// RemoveRepresentation.
// \li argument_type : identifies the type for value passed to the method on the
// VTK object. Accepted values are "VTK", "SMProxy" or "SIProxy". Default is
// VTK.
// \li null_on_empty : if set to 1, whenever the property's value changes to
// empty i.e. it contains no proxies, the command is called on the VTK object
// with NULL argument useful when there's no clean_command that can be called on
// the VTK object to unset the property e.g. SetLookupTable(NULL).
// li skip_dependency : if set to 1, this property does not result in adding a
// dependency between the proxies set as values of this property and the proxy
// to which the property belongs (which is the default behaviour). Use this with
// care as it would mean that ParaView would no realize any updates are needed
// to the pipeline if any proxy set on the property changes. This is necessary
// in some cases, e.g. if LUT proxy on a representation changes, we don't want
// to representation to treat it same as if the input pipeline changed!
// .SECTION See Also
// vtkSMProperty

#ifndef vtkSMProxyProperty_h
#define vtkSMProxyProperty_h

#include "vtkPVServerManagerCoreModule.h" //needed for exports
#include "vtkSMProperty.h"

class vtkSMProxy;
class vtkSMStateLocator;

class VTKPVSERVERMANAGERCORE_EXPORT vtkSMProxyProperty : public vtkSMProperty
{
public:
  // Description:
  // When we load ProxyManager state we want Proxy/InputProperty to be able to
  // create the corresponding missing proxy. Although when the goal is to load
  // a state on any standard proxy, we do not want that proxy property be able
  // to create new proxy based on some previous state.
  static void EnableProxyCreation();
  static void DisableProxyCreation();
  static bool CanCreateProxy();

  static vtkSMProxyProperty* New();
  vtkTypeMacro(vtkSMProxyProperty, vtkSMProperty);
  void PrintSelf(ostream& os, vtkIndent indent);

  // Description:
  // Add/remove/set a proxy to the list of proxies. For SetProxy, the property
  // automatically resizes to accommodate the index specified.
  virtual void AddProxy(vtkSMProxy* proxy);
  virtual void SetProxy(unsigned int idx, vtkSMProxy* proxy);
  virtual void RemoveProxy(vtkSMProxy* proxy);
  virtual void RemoveAllProxies();

  // Description:
  // Sets the value of the property to the list of proxies specified.
  virtual void SetProxies(unsigned int numElements, vtkSMProxy* proxies[]);

  // Description:
  // Returns if the given proxy is already added to the property.
  bool IsProxyAdded(vtkSMProxy* proxy);

  // Description:
  // Add an unchecked proxy. Does not modify the property.
  // Unchecked proxies are used by domains when verifying whether
  // a value is acceptable. To check if a value is in the domains,
  // you can do the following:
  // @verbatim
  // - RemoveAllUncheckedProxies()
  // - AddUncheckedProxy(proxy)
  // - IsInDomains()
  // @endverbatim
  virtual void AddUncheckedProxy(vtkSMProxy* proxy);
  virtual void SetUncheckedProxy(unsigned int idx, vtkSMProxy* proxy);

  // Description:
  // Removes all unchecked proxies.
  virtual void RemoveAllUncheckedProxies();

  // Description:
  // Returns the number of proxies.
  unsigned int GetNumberOfProxies();

  // Description:
  // Returns the number of unchecked proxies.
  unsigned int GetNumberOfUncheckedProxies();

  // Description:
  // Set the number of proxies.
  void SetNumberOfProxies(unsigned int count);
  void SetNumberOfUncheckedProxies(unsigned int count);

  // Description:
  // Return a proxy. No bounds check is performed.
  vtkSMProxy* GetProxy(unsigned int idx);

  // Description:
  // Return a proxy. No bounds check is performed.
  vtkSMProxy* GetUncheckedProxy(unsigned int idx);

  // Description:
  // Copy all property values.
  virtual void Copy(vtkSMProperty* src);

  // Description:
  // Returns whether the "skip_dependency" attribute is set.
  vtkGetMacro(SkipDependency, bool);

  // Description:
  // Update all proxies referred by this property (if any).
  virtual void UpdateAllInputs();

  virtual bool IsValueDefault();

  // Description:
  // For properties that support specifying defaults in XML configuration, this
  // method will reset the property value to the default values specified in the
  // XML.
  // Simply clears the property.
  virtual void ResetToXMLDefaults();

//BTX
protected:
  vtkSMProxyProperty();
  ~vtkSMProxyProperty();

  // Description:
  // Let the property write its content into the stream
  virtual void WriteTo(vtkSMMessage* msg);

  // Description:
  // Let the property read and set its content from the stream
  virtual void ReadFrom(const vtkSMMessage* msg, int msg_offset, vtkSMProxyLocator*);

  friend class vtkSMProxy;

  // Description:
  // Set the appropriate ivars from the xml element. Should
  // be overwritten by subclass if adding ivars.
  virtual int ReadXMLAttributes(vtkSMProxy* parent, 
                                vtkPVXMLElement* element);


  // Description:
  // Generic method used to generate XML state
  virtual void SaveStateValues(vtkPVXMLElement* propertyElement);

  // Description:
  // Fill state property/proxy XML element with proxy info.
  // Return the created proxy XML element that has been added as a child in the
  // property definition. If prop == NULL, you must Delete yourself the result
  // otherwise prop is olding a reference to the proxy element
  virtual vtkPVXMLElement* AddProxyElementState(vtkPVXMLElement *prop,
                                                unsigned int idx);
  // Description:
  // Updates state from an XML element. Returns 0 on failure.
  virtual int LoadState(vtkPVXMLElement* element, vtkSMProxyLocator* loader);


  // Description:
  // Called when a producer fires the vtkCommand::UpdateDataEvent. We update all
  // dependent domains since the data-information may have changed.
  void OnUpdateDataEvent() { this->UpdateDomains(); }

  // Static flag used to know if the locator should be used to create proxy
  // or if the session should be used to find only the existing ones
  static bool CreateProxyAllowed;

  bool SkipDependency;

  class vtkPPInternals;
  friend class vtkPPInternals;
  vtkPPInternals* PPInternals;
private:
  vtkSMProxyProperty(const vtkSMProxyProperty&); // Not implemented
  void operator=(const vtkSMProxyProperty&); // Not implemented
//ETX
};

#endif
