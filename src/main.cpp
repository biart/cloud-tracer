#include <application.h>


namespace ct
{
    class CloudTracerApplication : public Application
    {
    public:
        CloudTracerApplication() : Application("Cloud Tracer") {}

    protected:
        virtual void Start() override
        {

        }

        virtual void Update() override
        {

        }

        virtual void Destroy() override
        {

        }
    };
}


int main()
{
    ct::CloudTracerApplication application;

    try
    {
        application.Run();
    }
    catch (const std::exception& e)
    {
        std::cout << "Exception occured: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}