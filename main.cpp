#include <iostream>
#include <stdarg.h>
#include <vector>
#include <AttestationClient.h>
#include <nlohmann/json.hpp>
#include <iostream>
#include <string>
#include <algorithm>
#include <thread>
#include <boost/algorithm/string.hpp>
#include "Utils.h"
#include "Logger.h"
using json = nlohmann::json;

#define OUTPUT_TYPE_JWT "TOKEN"
#define OUTPUT_TYPE_BOOL "BOOL"

// default guest attestation url
std::string attestation_url = "https://sharedeus2.eus2.attest.azure.net/";

int main(int argc, char* argv[]) {
    std::string nonce;

    try {
        AttestationClient* attestation_client = nullptr;
        Logger* log_handle = new Logger();

        // Initialize attestation client
        if (!Initialize(log_handle, &attestation_client)) {
            printf("Failed to create attestation client object\n");
            Uninitialize();
            exit(1);
        }

        // parameters for the Attest call       
        attest::ClientParameters params = {};
        params.attestation_endpoint_url = (unsigned char*)attestation_url.c_str();
        std::string client_payload_str = "{\"nonce\":\"" + nonce + "\"}"; // nonce is optional
        params.client_payload = (unsigned char*) client_payload_str.c_str();
        params.version = CLIENT_PARAMS_VERSION;
        unsigned char* jwt = nullptr;
        attest::AttestationResult result;
        
        bool is_cvm = false;
        bool attestation_success = true;
        std::string jwt_str;
        // call attest
        if ((result = attestation_client->Attest(params, &jwt)).code_ 
                != attest::AttestationResult::ErrorCode::SUCCESS) {
            attestation_success = false;
        }

        if (attestation_success) {
            jwt_str = reinterpret_cast<char*>(jwt);
            attestation_client->Free(jwt);
            // Prase attestation token to extract isolation tee details
            std::vector<std::string> tokens;
            boost::split(tokens, jwt_str, [](char c) {return c == '.'; });
            if (tokens.size() < 3) {
                printf("Invalid JWT token");
                exit(1);
            }

            json attestation_claims = json::parse(base64_decode(tokens[1]));
            try {
                std::string attestation_type 
                    = attestation_claims["x-ms-isolation-tee"]["x-ms-attestation-type"].get<std::string>();
                std::string compliance_status 
                    = attestation_claims["x-ms-isolation-tee"]["x-ms-compliance-status"].get<std::string>();
                if (boost::iequals(attestation_type, "sevsnpvm") &&
                    boost::iequals(compliance_status, "azure-compliant-cvm")){
                    is_cvm = true;
                }
            }
            catch (...) { } // sevsnp claim does not exist in the token
        }

        printf("%s", is_cvm ? "true" : "false");

        Uninitialize();
    }
    catch (std::exception& e) {
        printf("Exception occurred. Details - %s", e.what());
        exit(1);
    }
}
