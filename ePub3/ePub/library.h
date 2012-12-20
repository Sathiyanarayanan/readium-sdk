//
//  library.h
//  ePub3
//
//  Created by Jim Dovey on 2012-12-14.
//  Copyright (c) 2012 Kobo Inc. All rights reserved.
//

#ifndef __ePub3__library__
#define __ePub3__library__

#include "epub3.h"
#include "container.h"
#include "Package.h"
#include "cfi.h"
#include "locator.h"
#include <map>
#include <atomic>

EPUB3_BEGIN_NAMESPACE

// Note that this is a library in the smallest sense: it keeps track of ePub files
//  by their unique-identifier, storing the path to that ePub file *as given*. Its
//  primary presence here is to allow for inter-publication linking. It is also
//  designed to be subclassed in order to serve as the basis of a complete library
//  in an application, with all the additional functionality that would involve.
//
// Note that each application would have a single library, accessible only through
//  the MainLibrary() static method. A library can optionally be written out to
//  disk and then loaded back in by calling MainLibrary() with a Locator instance
//  at application startup. Once the singleton instance has been created,
//  MainLibrary() will ignore its argument and always return that instance.
//
// Thoughts: OCF allows for multiple packages to be specified, but I don't see any
//  handling of that in ePub3 CFI?

class Library
{
public:
    typedef std::string     EPubIdentifier;
    
protected:
    Library() = default;
    Library(const Library&) = default;
    Library(Library&& o) : _containers(std::move(o._containers)), _packages(std::move(o._packages)) {}
    
    // load a library from a file generated using WriteToFile()
    Library(Locator* locator);
    bool Load(Locator* locator);
    
public:
    // access a singleton instance managed by the class
    static Shared<Library> MainLibrary(Locator* locator = new NullLocator());
    virtual ~Library() { _singleton.reset(); }
    
    Locator LocatorForEPubWithUniqueID(const std::string& uniqueID) const;
    
    void AddEPubsInContainer(Shared<Container> container, Locator* locator = new NullLocator());
    void AddEPubsInContainerAtPath(Locator* locator) {
        Container * c = new Container(*locator);
        return AddEPubsInContainer(Shared<Container>(c));
    }
    
    // returns an epub3:// url for the package with a given identifier
    std::string EPubURLForPackage(Shared<const Package> package) const;
    std::string EPubURLForPackage(const std::string& identifier) const;
    
    // may load a container/package, so non-const
    Package* PackageForEPubURL(const std::string& url);
    
    std::string EPubCFIURLForManifestItem(Shared<const ManifestItem> item);
    
    // may instantiate a Container & store it, so non-const
    const ManifestItem* ManifestItemForCFI(const std::string& urlWithCFI);
    
    // file format is sort-of CSV
    // each line starts with a container locator's string representation followed by a
    //  comma-separated list of package identifiers
    bool WriteToFile(Locator* locator) const;
    
protected:
    // list of known (but not necessarily loaded) containers
    typedef std::map<Shared<Locator>, Shared<Container>>    ContainerLookup;
    
    // if container is loaded, LookupEntry will contain a Package
    // otherwise, the locator is used to load the Container
    typedef std::pair<Shared<Locator>, Shared<Package>>     LookupEntry;
    typedef std::map<EPubIdentifier, LookupEntry>           PackageLookup;
    
    ContainerLookup _containers;
    PackageLookup   _packages;
    
    static Shared<Library>  _singleton;
};

EPUB3_END_NAMESPACE

#endif /* defined(__ePub3__library__) */