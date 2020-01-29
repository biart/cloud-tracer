#pragma once


namespace ct
{
    namespace utils
    {
        inline void IgnoreUnused();

        template <typename Car, typename ...Cdr>
        void IgnoreUnused(Car& first_unused_variable, const Cdr& ... rest_unused_variables);
    }
}


void ct::utils::IgnoreUnused()
{
}


template <typename Car, typename ...Cdr>
void ct::utils::IgnoreUnused(Car& first_unused_variable, const Cdr& ... rest_unused_variables)
{
    first_unused_variable;
    IgnoreUnused(rest_unused_variables...);
}
