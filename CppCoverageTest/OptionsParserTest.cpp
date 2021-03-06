// OpenCppCoverage is an open source code coverage for C++.
// Copyright (C) 2014 OpenCppCoverage
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include "stdafx.h"

#include <boost/filesystem.hpp>
#include "CppCoverage/Options.hpp"
#include "CppCoverage/ProgramOptions.hpp"

#include "CppCoverageTest/TestTools.hpp"

#include "ToolsCross/Tool.hpp"
#include "TestHelper/TemporaryPath.hpp"

namespace cov = CppCoverage;
namespace fs = boost::filesystem;

namespace CppCoverageTest
{
	namespace
	{
		const std::string optionShortPrefix = "-";		
	}
		
	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, Default)
	{
		cov::OptionsParser parser;

		auto options = TestTools::Parse(parser, {});
		ASSERT_TRUE(static_cast<bool>(options));
		ASSERT_EQ(cov::LogLevel::Normal, options->GetLogLevel());
		ASSERT_FALSE(options->IsPlugingModeEnabled());
		ASSERT_FALSE(options->IsCoverChildrenModeEnabled());
		ASSERT_TRUE(options->IsAggregateByFileModeEnabled());
		ASSERT_FALSE(options->IsContinueAfterCppExceptionModeEnabled());
		ASSERT_FALSE(options->IsOptimizedBuildSupportEnabled());
		ASSERT_TRUE(options->GetExcludedLineRegexes().empty());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, Help)
	{
		cov::OptionsParser parser;
		std::wostringstream ostr;

		ASSERT_FALSE(static_cast<bool>(TestTools::Parse(parser,
		{ optionShortPrefix + cov::ProgramOptions::HelpShortOption }, false, &ostr)));
		
		ASSERT_NE(L"", ostr.str());
		ASSERT_FALSE(static_cast<bool>(TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::HelpOption }, false)));
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, Verbose)
	{
		cov::OptionsParser parser;

		ASSERT_EQ(cov::LogLevel::Verbose, TestTools::Parse(parser,
			{ optionShortPrefix + cov::ProgramOptions::VerboseShortOption })->GetLogLevel());
		ASSERT_EQ(cov::LogLevel::Verbose, TestTools::Parse(parser,
			{ TestTools::OptionPrefix + cov::ProgramOptions::VerboseOption })->GetLogLevel());
	}
	
	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, Quiet)
	{
		cov::OptionsParser parser;

		ASSERT_EQ(cov::LogLevel::Quiet, TestTools::Parse(parser,
		{ optionShortPrefix + cov::ProgramOptions::QuietShortOption })->GetLogLevel());
		ASSERT_EQ(cov::LogLevel::Quiet, TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::QuietOption })->GetLogLevel());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, Plugin)
	{
		cov::OptionsParser parser;

		ASSERT_TRUE(TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::PluginOption })->IsPlugingModeEnabled());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, CoverChildren)
	{
		cov::OptionsParser parser;

		ASSERT_TRUE(TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::CoverChildrenOption })->IsCoverChildrenModeEnabled());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, FileAggregate)
	{
		cov::OptionsParser parser;

		ASSERT_FALSE(TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::NoAggregateByFileOption })->IsAggregateByFileModeEnabled());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, ContinueAfterCppException)
	{
		cov::OptionsParser parser;

		ASSERT_TRUE(TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::ContinueAfterCppExceptionOption })
			->IsContinueAfterCppExceptionModeEnabled());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, WorkingDirectory)
	{
		cov::OptionsParser parser;
		const std::string folder = ".";

		auto options = TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::WorkingDirectoryOption, folder });
		ASSERT_TRUE(static_cast<bool>(options));
		ASSERT_NE(nullptr, options->GetStartInfo());

		const auto* workingDirectory = options->GetStartInfo()->GetWorkingDirectory();

		ASSERT_NE(nullptr, workingDirectory);
		ASSERT_EQ(Tools::LocalToWString(folder), *workingDirectory);
	}
		
	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, Program)
	{
		cov::OptionsParser parser;		
		const std::string arg1 = "arg1";
		const std::string arg2 = "arg2";
		const std::vector<std::wstring> expectedArgs =
			{	Tools::LocalToWString(TestTools::ProgramToRun), 
				Tools::LocalToWString(arg1), 
				Tools::LocalToWString(arg2) };

		auto options = TestTools::Parse(parser, { TestTools::ProgramToRun, arg1, arg2 }, false);
		ASSERT_TRUE(static_cast<bool>(options));

		const auto* startInfo = options->GetStartInfo();		
		ASSERT_NE(nullptr, startInfo);
		ASSERT_EQ(TestTools::ProgramToRun, startInfo->GetPath().string());
		ASSERT_EQ(expectedArgs, startInfo->GetArguments());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, UnknownOption)
	{
		cov::OptionsParser parser;
		std::wostringstream ostr;
		ASSERT_FALSE(TestTools::Parse(parser, { "--unknownOption" }, true, &ostr));
		ASSERT_NE(L"", ostr.str());
	}
				
	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, OptionOstream)
	{ 
		cov::OptionsParser parser;
		auto options = TestTools::Parse(parser, { TestTools::OptionPrefix + cov::ProgramOptions::SelectedSourcesOption, "source1" });
		
		std::wostringstream ostr;

		ASSERT_TRUE(static_cast<bool>(options));
		ostr << *options;
		ASSERT_LT(L"", ostr.str());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, InputCoverage)
	{
		cov::OptionsParser parser;		
		TestHelper::TemporaryPath temporaryPath{ TestHelper::TemporaryPathOption::CreateAsFile };
		auto pathStr = temporaryPath.GetPath().string();

		auto options = TestTools::Parse(parser, 
			{ TestTools::OptionPrefix + cov::ProgramOptions::InputCoverageValue, pathStr });
		ASSERT_TRUE(static_cast<bool>(options));
		ASSERT_EQ(pathStr, options->GetInputCoveragePaths().at(0).string());		
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, InvalidInputCoverage)
	{
		cov::OptionsParser parser;
		std::wostringstream ostr;
		ASSERT_FALSE(static_cast<bool>(TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::InputCoverageValue, "invalidPath" }, true, &ostr)));
		ASSERT_NE(L"", ostr.str());		
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, OptimizedBuild)
	{
		cov::OptionsParser parser;

		ASSERT_TRUE(TestTools::Parse(parser,
		{ TestTools::OptionPrefix + cov::ProgramOptions::OptimizedBuildOption })
			->IsOptimizedBuildSupportEnabled());
	}

	//-------------------------------------------------------------------------
	TEST(OptionsParserTest, ExcludedLineRegex)
	{
		cov::OptionsParser parser;
		const auto excludedLineRegex = ".*";
		auto option = TestTools::Parse(parser, 
			{	TestTools::OptionPrefix + cov::ProgramOptions::ExcludedLineRegexOption,
				excludedLineRegex });
		ASSERT_TRUE(option.is_initialized());
		ASSERT_THAT(
			option->GetExcludedLineRegexes(), 
			testing::ElementsAre(Tools::LocalToWString(excludedLineRegex)));
	}
}