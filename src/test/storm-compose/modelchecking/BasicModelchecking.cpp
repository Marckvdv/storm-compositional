#include "storm-config.h"
#include "test/storm_gtest.h"
#include "storm/api/storm.h"
#include "storm-compose/parser/JsonStringDiagramParser.h"
#include "storm-compose/models/OpenMdp.h"
#include "storm-compose/modelchecker/MonolithicOpenMdpChecker.h"
#include "storm-compose/modelchecker/NaiveOpenMdpChecker2.h"
#include "storm-compose/benchmark/BenchmarkStats.h"

class DoubleEnvironment {
   public:
    typedef double ValueType;
    static storm::Environment createEnvironment() {
        storm::Environment env;
        return env;
    }

    static storm::models::visitor::LowerUpperParetoSettings createLowerUpperSettings() {
        storm::models::visitor::LowerUpperParetoSettings settings;
        settings.precision = 1e-6;
        settings.precisionType = "absolute";
        settings.steps = {};
        return settings;
    }
};

template<typename TestType>
class BasicModelcheckingTest : public ::testing::Test {
   public:
    typedef typename TestType::ValueType ValueType;
    BasicModelcheckingTest() : _environment(TestType::createEnvironment()), _lowerUpperParetoSettings(TestType::createLowerUpperSettings()) {}
    storm::Environment const& env() const {
        return _environment;
    }
    storm::models::visitor::LowerUpperParetoSettings const& lowerUpperSettings() const {
        return _lowerUpperParetoSettings;
    }
    ValueType parseNumber(std::string const& str) {
        return storm::utility::convertNumber<ValueType>(str);
    }

    struct Input {
        std::shared_ptr<storm::models::OpenMdpManager<ValueType>> manager;
    };

    Input buildPrism(std::string const& programFile) const {
        auto omdpManager = std::make_shared<storm::models::OpenMdpManager<ValueType>>();
        auto parser = storm::parser::JsonStringDiagramParser<ValueType>::fromFilePath(programFile, omdpManager);
        parser.parse();

        return { omdpManager };
    }

   private:
    storm::Environment _environment;
    storm::models::visitor::LowerUpperParetoSettings _lowerUpperParetoSettings;
};

typedef ::testing::Types<DoubleEnvironment> TestingTypes;

TYPED_TEST_SUITE(BasicModelcheckingTest, TestingTypes, );

TYPED_TEST(BasicModelcheckingTest, MonolithicRightwardReachability) {
    using namespace storm::modelchecker;
    using namespace storm::compose::benchmark;

    const static std::vector<std::string> cases {
        "test1/sd.json"
    };

    OpenMdpReachabilityTask task;
    typedef typename TestFixture::ValueType ValueType;
    for (const auto& path : cases) {
        auto input = this->buildPrism(STORM_TEST_RESOURCES_DIR "/compose/" + path);
        auto& manager = input.manager;
        auto model = manager->getRoot();

        BenchmarkStats<ValueType> stats;
        MonolithicOpenMdpChecker<ValueType> monolithicChecker(manager, stats);

        auto monolithicResult = monolithicChecker.check(task).getLowerBound();

        NaiveOpenMdpChecker2<ValueType> naiveChecker(manager, stats, this->lowerUpperSettings());
        auto naiveResult = naiveChecker.check(task);

        EXPECT_TRUE(naiveResult.getLowerBound() <= monolithicResult && naiveResult.getUpperBound() >= monolithicResult);
    }
}
