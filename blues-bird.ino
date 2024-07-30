#include <Seeed_Arduino_SSCMA.h>
#include <Notecard.h>

SSCMA AI;
Notecard notecard;

#define accuracyThreshold 95
#define tiltPin D7
#define productUID "PRODUCT_UID_HERE"

String birdNames[] = {
    "AMERICAN AVOCET",
    "AMERICAN BITTERN",
    "AMERICAN COOT",
    "AMERICAN FLAMINGO",
    "AMERICAN GOLDFINCH",
    "AMERICAN KESTREL",
    "AMERICAN PIPIT",
    "AMERICAN REDSTART",
    "AMERICAN ROBIN",
    "AMERICAN WIGEON",
    "BALD EAGLE",
    "BELTED KINGFISHER",
    "BLACK SKIMMER",
    "BOBOLINK",
    "BROWN HEADED COWBIRD",
    "BROWN THRASHER",
    "CEDAR WAXWING",
    "CHIPPING SPARROW",
    "COMMON GRACKLE",
    "CROW",
    "DOUBLE BRESTED CORMARANT",
    "DUNLIN",
    "EASTERN BLUEBIRD",
    "EASTERN MEADOWLARK",
    "EASTERN TOWEE",
    "MOURNING DOVE",
    "NORTHERN CARDINAL",
    "NORTHERN MOCKINGBIRD",
    "PAINTED BUNTING",
    "RED HEADED WOODPECKER",
    "RED SHOULDERED HAWK",
    "RUBY CROWNED KINGLET",
    "RUBY THROATED HUMMINGBIRD",
    "SCARLET CROWNED FRUIT DOVE",
    "SCARLET TANAGER",
    "SNOWY EGRET",
    "VICTORIA CROWNED PIGEON",
    "WOOD DUCK"
};

void setup()
{
    Serial.begin(9600);

    // Set tiltPin as an input pin
    pinMode(tiltPin, INPUT); 

    // Initialize Seeed Grove Vision AI V2 via SSCMA
    AI.begin();

    // Initialize notecard
    notecard.begin();

    J *req = notecard.newRequest("hub.set");
    if (req) {
        JAddStringToObject(req, "product", productUID);
        JAddStringToObject(req, "mode", "periodic");
        JAddNumberToObject(req, "outbound", 5);
        JAddNumberToObject(req, "inbound", 60);
        if (!notecard.sendRequest(req)) {
            notecard.logDebug("FATAL: Failed to configure Notecard!\n");
            while(1);
        }
    }
}

void loop()
{
    // If the tilt pin reads 0 (in this case indicating movement for this sensor)
    if (digitalRead(tiltPin) == 0) {
        // Invoke SSCMA asking inferencing and returning the image (third parameter true)
        if (!AI.invoke(1, false, true))
        {
            // Debug logic
            Serial.println("invoke success");
            Serial.print("perf: prepocess=");
            Serial.print(AI.perf().prepocess);
            Serial.print(", inference=");
            Serial.print(AI.perf().inference);
            Serial.print(", postpocess=");
            Serial.println(AI.perf().postprocess);

            for (int i = 0; i < AI.classes().size(); i++)
            {
                if (AI.classes()[i].score >= accuracyThreshold) {
                    Serial.print("Class[");
                    Serial.print(i);
                    Serial.print("] target=");
                    Serial.print(AI.classes()[i].target);
                    Serial.print(", score=");
                    Serial.println(AI.classes()[i].score);

                    Serial.print("Image:");
                    Serial.println();

                    J *req = notecard.newRequest("note.add");
                    if (req != NULL)
                    {
                        JAddStringToObject(req, "file", "events.qo");
                        J *body = JAddObjectToObject(req, "body");
                        if (body)
                        {
                            JAddStringToObject(body, "bird", birdNames[AI.classes()[i].target].c_str());
                        }
                        JAddStringToObject(req, "payload", AI.last_image().c_str());
                        notecard.sendRequest(req);
                    }
                }
            }
        }
    }
}