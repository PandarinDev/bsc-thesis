#pragma once

namespace inf::wfc {

    template<typename ContextType, typename InstanceType>
    struct Rule {

        float weight;

        Rule() : weight(1.0f) {}
        Rule(float weight) : weight(weight) {}
        virtual ~Rule() = default;

        virtual bool matches(const ContextType& context, const InstanceType& instance) = 0;
        virtual void apply(InstanceType& instance) = 0;

    };

}