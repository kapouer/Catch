/*
 *  Created by Phil on 22/10/2010.
 *  Copyright 2010 Two Blue Cubes Ltd
 *
 *  Distributed under the Boost Software License, Version 1.0. (See accompanying
 *  file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
 */
#ifdef __clang__
#pragma clang diagnostic ignored "-Wpadded"
#endif

#include "catch_self_test.hpp"
#include "internal/catch_text.h"
#include "internal/catch_console_colour.hpp"

TEST_CASE( "selftest/main", "Runs all Catch self tests and checks their results" ) {
    using namespace Catch;

    ///////////////////////////////////////////////////////////////////////////
    SECTION(    "selftest/expected result",
                "Tests do what they claim" ) {
                
        SECTION(    "selftest/expected result/failing tests", 
                    "Tests in the 'failing' branch fail" ) {
            MetaTestRunner::runMatching( "./failing/*",  MetaTestRunner::Expected::ToFail, 0, 2 );
        }
        
        SECTION(    "selftest/expected result/succeeding tests", 
                    "Tests in the 'succeeding' branch succeed" ) {
            MetaTestRunner::runMatching( "./succeeding/*",  MetaTestRunner::Expected::ToSucceed, 1, 2 );
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    SECTION(    "selftest/test counts", 
                "Number of test cases that run is fixed" ) {
        EmbeddedRunner runner;
        
        SECTION(    "selftest/test counts/succeeding tests", 
                    "Number of 'succeeding' tests is fixed" ) {
            Totals totals = runner.runMatching( "./succeeding/*", 0, 2 );
            CHECK( totals.assertions.passed == 298 );
            CHECK( totals.assertions.failed == 0 );
        }

        SECTION(    "selftest/test counts/failing tests", 
                    "Number of 'failing' tests is fixed" ) {
            Totals totals = runner.runMatching( "./failing/*", 1, 2 );
            CHECK( totals.assertions.passed == 2 );
            CHECK( totals.assertions.failed == 77 );
        }
    }
}

TEST_CASE( "meta/Misc/Sections", "looped tests" ) {
    Catch::EmbeddedRunner runner;
    
    Catch::Totals totals = runner.runMatching( "./mixed/Misc/Sections/nested2", 0, 1 );
    CHECK( totals.assertions.passed == 2 );
    CHECK( totals.assertions.failed == 1 );
}

#ifdef __clang__
#pragma clang diagnostic ignored "-Wweak-vtables"
#endif

#include "../../include/internal/catch_commandline.hpp"
#include "../../include/internal/catch_test_spec.h"
#include "../../include/reporters/catch_reporter_xml.hpp"

template<size_t size>
void parseIntoConfig( const char * (&argv)[size], Catch::ConfigData& config ) {
    Clara::CommandLine<Catch::ConfigData> parser = Catch::makeCommandLineParser();
    parser.parseInto( size, argv, config );
}

template<size_t size>
std::string parseIntoConfigAndReturnError( const char * (&argv)[size], Catch::ConfigData& config ) {
    try {
        parseIntoConfig( argv, config );
        FAIL( "expected exception" );
    }
    catch( std::exception& ex ) {
        return ex.what();
    }
    return "";
}

inline Catch::TestCase fakeTestCase( const char* name ){ return Catch::makeTestCase( NULL, "", name, "", CATCH_INTERNAL_LINEINFO ); }

TEST_CASE( "Process can be configured on command line", "[config][command-line]" ) {

    Catch::ConfigData config;

    SECTION( "default - no arguments", "" ) {
        const char* argv[] = { "test" };
        CHECK_NOTHROW( parseIntoConfig( argv, config ) );
        
        CHECK( config.shouldDebugBreak == false );
        CHECK( config.abortAfter == -1 );
        CHECK( config.noThrow == false );
        CHECK( config.reporterName.empty() );
    }
    
    SECTION( "test lists", "" ) {
        SECTION( "1 test", "Specify one test case using" ) {
            const char* argv[] = { "test", "test1" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            Catch::Config cfg( config );
            REQUIRE( cfg.filters().size() == 1 );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "notIncluded" ) ) == false );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "test1" ) ) );
        }
        SECTION( "Specify one test case exclusion using exclude:", "" ) {
            const char* argv[] = { "test", "exclude:test1" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            Catch::Config cfg( config );
            REQUIRE( cfg.filters().size() == 1 );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "test1" ) ) == false );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "alwaysIncluded" ) ) );
        }

        SECTION( "Specify one test case exclusion using ~", "" ) {
            const char* argv[] = { "test", "~test1" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            Catch::Config cfg( config );
            REQUIRE( cfg.filters().size() == 1 );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "test1" ) ) == false );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "alwaysIncluded" ) ) );
        }
        
        SECTION( "Specify two test cases using -t", "" ) {
            const char* argv[] = { "test", "-t", "test1", "test2" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            Catch::Config cfg( config );
            REQUIRE( cfg.filters().size() == 1 );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "notIncluded" ) ) == false );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "test1" ) ) );
            REQUIRE( cfg.filters()[0].shouldInclude( fakeTestCase( "test2" ) ) );
        }
    }
    
    SECTION( "reporter", "" ) {
        SECTION( "-r/console", "" ) {
            const char* argv[] = { "test", "-r", "console" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.reporterName == "console" );
        }
        SECTION( "-r/xml", "" ) {
            const char* argv[] = { "test", "-r", "xml" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.reporterName == "xml" );
        }
        SECTION( "--reporter/junit", "" ) {
            const char* argv[] = { "test", "--reporter", "junit" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.reporterName == "junit" );
        }
    }
    
    SECTION( "debugger", "" ) {
        SECTION( "-b", "" ) {
            const char* argv[] = { "test", "-b" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.shouldDebugBreak == true );
        }
        SECTION( "--break", "" ) {
            const char* argv[] = { "test", "--break" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );
            
            REQUIRE( config.shouldDebugBreak );
        }
    }
        
    SECTION( "abort", "" ) {
        SECTION( "-a aborts after first failure", "" ) {
            const char* argv[] = { "test", "-a" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.abortAfter == 1 );
        }
        SECTION( "-x 2 aborts after two failures", "" ) {
            const char* argv[] = { "test", "-x", "2" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.abortAfter == 2 );
        }
        SECTION( "-x must be greater than zero", "" ) {
            const char* argv[] = { "test", "-x", "0" };
            REQUIRE_THAT( parseIntoConfigAndReturnError( argv, config ), Contains( "greater than zero" ) );
        }
        SECTION( "-x must be numeric", "" ) {
            const char* argv[] = { "test", "-x", "oops" };
            REQUIRE_THAT( parseIntoConfigAndReturnError( argv, config ), Contains( "-x" ) );
        }
    }
    
    SECTION( "nothrow", "" ) {
        SECTION( "-e", "" ) {
            const char* argv[] = { "test", "-e" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.noThrow == true );
        }
        SECTION( "--nothrow", "" ) {
            const char* argv[] = { "test", "--nothrow" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.noThrow == true );
        }
    }

    SECTION( "output filename", "" ) {
        SECTION( "-o filename", "" ) {
            const char* argv[] = { "test", "-o", "filename.ext" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.outputFilename == "filename.ext" );
        }
        SECTION( "--out", "" ) {
            const char* argv[] = { "test", "--out", "filename.ext" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            REQUIRE( config.outputFilename == "filename.ext" );
        }
    }

    SECTION( "combinations", "" ) {
        SECTION( "Single character flags can be combined", "" ) {
            const char* argv[] = { "test", "-abe" };
            CHECK_NOTHROW( parseIntoConfig( argv, config ) );

            CHECK( config.abortAfter == 1 );
            CHECK( config.shouldDebugBreak );
            CHECK( config.noThrow == true );
        }
    }        
}

TEST_CASE( "selftest/test filter", "Individual filters" ) {

    Catch::TestCaseFilter matchAny( "*" );
    Catch::TestCaseFilter matchNone( "*", Catch::IfFilterMatches::ExcludeTests );
    CHECK( matchAny.shouldInclude( fakeTestCase( "any" ) ));
    CHECK( matchNone.shouldInclude( fakeTestCase( "any" ) ) == false );

    Catch::TestCaseFilter matchHidden( "./*" );
    Catch::TestCaseFilter matchNonHidden( "./*", Catch::IfFilterMatches::ExcludeTests );

    CHECK( matchHidden.shouldInclude( fakeTestCase( "any" ) ) == false );
    CHECK( matchNonHidden.shouldInclude( fakeTestCase( "any" ) ) );

    CHECK( matchHidden.shouldInclude( fakeTestCase( "./any" ) ) );
    CHECK( matchNonHidden.shouldInclude( fakeTestCase( "./any" ) ) == false );
}

TEST_CASE( "selftest/test filters", "Sets of filters" ) {

    Catch::TestCaseFilter matchHidden( "./*" );
    Catch::TestCaseFilter dontMatchA( "./a*", Catch::IfFilterMatches::ExcludeTests );
    Catch::TestCaseFilters filters( "" );
    filters.addFilter( matchHidden );
    filters.addFilter( dontMatchA );

    CHECK( matchHidden.shouldInclude( fakeTestCase( "./something" ) ) );

    CHECK( filters.shouldInclude( fakeTestCase( "any" ) ) == false );
    CHECK( filters.shouldInclude( fakeTestCase( "./something" ) ) );
    CHECK( filters.shouldInclude( fakeTestCase( "./anything" ) ) == false );
}

TEST_CASE( "selftest/filter/prefix wildcard", "Individual filters with wildcards at the start" ) {
    Catch::TestCaseFilter matchBadgers( "*badger" );

    CHECK( matchBadgers.shouldInclude( fakeTestCase( "big badger" ) ));
    CHECK( matchBadgers.shouldInclude( fakeTestCase( "little badgers" ) ) == false );
}
TEST_CASE( "selftest/filter/wildcard at both ends", "Individual filters with wildcards at both ends" ) {
    Catch::TestCaseFilter matchBadgers( "*badger*" );

    CHECK( matchBadgers.shouldInclude( fakeTestCase( "big badger" ) ));
    CHECK( matchBadgers.shouldInclude( fakeTestCase( "little badgers" ) ) );
    CHECK( matchBadgers.shouldInclude( fakeTestCase( "badgers are big" ) ) );
    CHECK( matchBadgers.shouldInclude( fakeTestCase( "hedgehogs" ) ) == false );
}


template<size_t size>
int getArgc( const char * (&)[size] ) {
    return size;
}

TEST_CASE( "selftest/tags", "" ) {

    std::string p1 = "[one]";
    std::string p2 = "[one],[two]";
    std::string p3 = "[one][two]";
    std::string p4 = "[one][two],[three]";
    std::string p5 = "[one][two]~[.],[three]";
    
    SECTION( "one tag", "" ) {
        Catch::TestCase oneTag = makeTestCase( NULL, "", "test", "[one]", CATCH_INTERNAL_LINEINFO );

        CHECK( oneTag.getTestCaseInfo().description == "" );
        CHECK( oneTag.hasTag( "one" ) );
        CHECK( oneTag.getTags().size() == 1 );

        CHECK( oneTag.matchesTags( p1 ) == true );
        CHECK( oneTag.matchesTags( p2 ) == true );
        CHECK( oneTag.matchesTags( p3 ) == false );
        CHECK( oneTag.matchesTags( p4 ) == false );
        CHECK( oneTag.matchesTags( p5 ) == false );
    }

    SECTION( "two tags", "" ) {
        Catch::TestCase twoTags= makeTestCase( NULL, "", "test", "[one][two]", CATCH_INTERNAL_LINEINFO );

        CHECK( twoTags.getTestCaseInfo().description == "" );
        CHECK( twoTags.hasTag( "one" ) );
        CHECK( twoTags.hasTag( "two" ) );
        CHECK( twoTags.hasTag( "Two" ) );
        CHECK( twoTags.hasTag( "three" ) == false );
        CHECK( twoTags.getTags().size() == 2 );

        CHECK( twoTags.matchesTags( p1 ) == true );
        CHECK( twoTags.matchesTags( p2 ) == true );
        CHECK( twoTags.matchesTags( p3 ) == true );
        CHECK( twoTags.matchesTags( p4 ) == true );
        CHECK( twoTags.matchesTags( p5 ) == true );
    }

    SECTION( "one tag with characters either side", "" ) {

        Catch::TestCase oneTagWithExtras = makeTestCase( NULL, "", "test", "12[one]34", CATCH_INTERNAL_LINEINFO );
        CHECK( oneTagWithExtras.getTestCaseInfo().description == "1234" );
        CHECK( oneTagWithExtras.hasTag( "one" ) );
        CHECK( oneTagWithExtras.hasTag( "two" ) == false );
        CHECK( oneTagWithExtras.getTags().size() == 1 );
    }
    
    SECTION( "start of a tag, but not closed", "" ) {

        Catch::TestCase oneTagOpen = makeTestCase( NULL, "", "test", "[one", CATCH_INTERNAL_LINEINFO );

        CHECK( oneTagOpen.getTestCaseInfo().description == "[one" );
        CHECK( oneTagOpen.hasTag( "one" ) == false );
        CHECK( oneTagOpen.getTags().size() == 0 );
    }

    SECTION( "hidden", "" ) {
        Catch::TestCase oneTag = makeTestCase( NULL, "", "test", "[.]", CATCH_INTERNAL_LINEINFO );

        CHECK( oneTag.getTestCaseInfo().description == "" );
        CHECK( oneTag.hasTag( "." ) );
        CHECK( oneTag.isHidden() );

        CHECK( oneTag.matchesTags( "~[.]" ) == false );

    }
}

TEST_CASE( "Long strings can be wrapped", "[wrap]" ) {

    using namespace Catch;
    SECTION( "plain string", "" ) {
        // guide:                 123456789012345678
        std::string testString = "one two three four";
        
        SECTION( "No wrapping", "" ) {
            CHECK( Text( testString, TextAttributes().setWidth( 80 ) ).toString() == testString );
            CHECK( Text( testString, TextAttributes().setWidth( 18 ) ).toString() == testString );
        }
        SECTION( "Wrapped once", "" ) {
            CHECK( Text( testString, TextAttributes().setWidth( 17 ) ).toString() == "one two three\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 16 ) ).toString() == "one two three\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 14 ) ).toString() == "one two three\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 13 ) ).toString() == "one two three\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 12 ) ).toString() == "one two\nthree four" );
        }
        SECTION( "Wrapped twice", "" ) {
            CHECK( Text( testString, TextAttributes().setWidth( 9 ) ).toString() == "one two\nthree\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 8 ) ).toString() == "one two\nthree\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 7 ) ).toString() == "one two\nthree\nfour" );
        }
        SECTION( "Wrapped three times", "" ) {
            CHECK( Text( testString, TextAttributes().setWidth( 6 ) ).toString() == "one\ntwo\nthree\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 5 ) ).toString() == "one\ntwo\nthree\nfour" );
        }
        SECTION( "Short wrap", "" ) {
            CHECK( Text( "abcdef", TextAttributes().setWidth( 4 ) ).toString() == "abc-\ndef" );
            CHECK( Text( "abcdefg", TextAttributes().setWidth( 4 ) ).toString() == "abc-\ndefg" );
            CHECK( Text( "abcdefgh", TextAttributes().setWidth( 4 ) ).toString() == "abc-\ndef-\ngh" );

            CHECK( Text( testString, TextAttributes().setWidth( 4 ) ).toString() == "one\ntwo\nthr-\nee\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 3 ) ).toString() == "one\ntwo\nth-\nree\nfo-\nur" );
        }
        SECTION( "As container", "" ) {
            Text text( testString, TextAttributes().setWidth( 6 ) );
            REQUIRE( text.size() == 4 );
            CHECK( text[0] == "one" );
            CHECK( text[1] == "two" );
            CHECK( text[2] == "three" );
            CHECK( text[3] == "four" );
        }
        SECTION( "Indent first line differently", "" ) {
            Text text( testString, TextAttributes()
                                        .setWidth( 10 )
                                        .setIndent( 4 )
                                        .setInitialIndent( 1 ) );
            CHECK( text.toString() == " one two\n    three\n    four" );
        }
        
    }
    
    SECTION( "With newlines", "" ) {
        
        // guide:                 1234567890123456789
        std::string testString = "one two\nthree four";
        
        SECTION( "No wrapping" , "" ) {
            CHECK( Text( testString, TextAttributes().setWidth( 80 ) ).toString() == testString );
            CHECK( Text( testString, TextAttributes().setWidth( 18 ) ).toString() == testString );
            CHECK( Text( testString, TextAttributes().setWidth( 10 ) ).toString() == testString );
        }
        SECTION( "Trailing newline" , "" ) {
            CHECK( Text( "abcdef\n", TextAttributes().setWidth( 10 ) ).toString() == "abcdef\n" );
            CHECK( Text( "abcdef", TextAttributes().setWidth( 6 ) ).toString() == "abcdef" );
            CHECK( Text( "abcdef\n", TextAttributes().setWidth( 6 ) ).toString() == "abcdef\n" );
        }
        SECTION( "Wrapped once", "" ) {
            CHECK( Text( testString, TextAttributes().setWidth( 9 ) ).toString() == "one two\nthree\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 8 ) ).toString() == "one two\nthree\nfour" );
            CHECK( Text( testString, TextAttributes().setWidth( 7 ) ).toString() == "one two\nthree\nfour" );
        }
        SECTION( "Wrapped twice", "" ) {
            CHECK( Text( testString, TextAttributes().setWidth( 6 ) ).toString() == "one\ntwo\nthree\nfour" );
        }
    }
    
    SECTION( "With tabs", "" ) {

        // guide:                 1234567890123456789
        std::string testString = "one two \tthree four five six";
        
        CHECK( Text( testString, TextAttributes().setWidth( 15 ) ).toString()
            == "one two three\n        four\n        five\n        six" );
    }
    
    
}

using namespace Catch;

class ColourString {
public:

    struct ColourIndex {
        ColourIndex( Colour::Code _colour, std::size_t _fromIndex, std::size_t _toIndex )
        :   colour( _colour ),
            fromIndex( _fromIndex ),
            toIndex( _toIndex )
        {}

        Colour::Code colour;
        std::size_t fromIndex;
        std::size_t toIndex;
    };

    ColourString( std::string const& _string )
    : string( _string )
    {}
    ColourString( std::string const& _string, std::vector<ColourIndex> const& _colours )
    : string( _string ), colours( _colours )
    {}
    
    ColourString& addColour( Colour::Code colour, int _index ) {
        colours.push_back( ColourIndex( colour,
                                        resolveRelativeIndex( _index ),
                                        resolveRelativeIndex( _index )+1 ) );
        return *this;
    }
    ColourString& addColour( Colour::Code colour, int _fromIndex, int _toIndex ) {
        colours.push_back( ColourIndex( colour,
                                        resolveRelativeIndex(_fromIndex),
                                        resolveLastRelativeIndex( _toIndex ) ) );
        return *this;
    }
    
    void writeToStream( std::ostream& _stream ) const {
        std::size_t last = 0;
        for( std::size_t i = 0; i < colours.size(); ++i ) {
            ColourIndex const& index = colours[i];
            if( index.fromIndex > last )
                _stream << string.substr( last, index.fromIndex-last );
            {
                Colour colourGuard( index.colour );
                _stream << string.substr( index.fromIndex, index.toIndex-index.fromIndex );
            }
            last = index.toIndex;
        }
        if( last < string.size() )
            _stream << string.substr( last );        
    }
    friend std::ostream& operator << ( std::ostream& _stream, ColourString const& _colourString ) {
        _colourString.writeToStream( _stream );
        return _stream;
    }

private:

    std::size_t resolveLastRelativeIndex( int _index ) {
        std::size_t index = resolveRelativeIndex( _index );
        return index == 0 ? string.size() : index;
    }
    std::size_t resolveRelativeIndex( int _index ) {
        return static_cast<std::size_t>( _index >= 0
            ? _index
            : static_cast<int>( string.size() )+_index );
    }
    std::string string;
    std::vector<ColourIndex> colours;
};

// !TBD: This will be folded into Text class
TEST_CASE( "Strings can be rendered with colour", "[colour]" ) {
    
    {
        ColourString cs( "hello" );
        cs  .addColour( Colour::Red, 0 )
            .addColour( Colour::Green, -1 );

        std::cout << cs << std::endl;
    }

    {
        ColourString cs( "hello" );
        cs  .addColour( Colour::Blue, 1, -2 );
        
        std::cout << cs << std::endl;
    }
    
}

TEST_CASE( "Text can be formatted using the Text class", "" ) {
    
    CHECK( Text( "hi there" ).toString() == "hi there" );
    
    TextAttributes narrow;
    narrow.setWidth( 6 );
    
    CHECK( Text( "hi there", narrow ).toString() == "hi\nthere" );
}
