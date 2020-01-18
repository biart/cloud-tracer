namespace ct
{
    namespace utils
    {
        void IgnoreUnused()
        {
        }


        template <typename Car, typename ...Cdr>
        void IgnoreUnused(Car& first_unused_variable, const Cdr&... rest_unused_variables)
        {
            first_unused_variable;
            IgnoreUnused(rest_unused_variables...);
        }
    }
}