import pyndnlite
import time


class OnInterest(pyndnlite.OnInterestFunc):
    def __init__(self):
        super().__init__()

    def handle(self, interest, interest_size):
        print("HIT")
        data = "\x06\x4e\x07\x0b\x08\x03\x6e\x64\x6e\x08\x04\x74\x65\x73\x74\x14\x03\x18\x01\x00" \
               "\x15\x13\x49\x27\x6d\x20\x61\x20\x44\x61\x74\x61\x20\x70\x61\x63\x6b\x65\x74\x2e" \
               "\x00\x16\x03\x1b\x01\x00\x17\x20\x87\x52\x49\xe4\x78\xd7\xad\x62\xd3\xa2\xf2\xaf" \
               "\xb6\x14\xfc\xb1\xb5\xac\x74\x5e\xd8\xc9\x28\xbe\x8a\xb9\xb5\x99\xb7\xa8\x89\x0a"
        datapacket = pyndnlite.byteArray(len(data))
        for i, v in enumerate(data):
            datapacket[i] = ord(v)
        pyndnlite.ndn_forwarder_put_data(datapacket, len(data))
        return pyndnlite.NDN_FWD_STRATEGY_MULTICAST


def main():
    running = True
    pyndnlite.ndn_lite_startup()
    # INADDR_ANY, 5000, 127.0.0.1, 6000
    pyndnlite.ndn_udp_unicast_face_construct(0, 34835, 16777343, 28695)
    name = "\x07\x0b\x08\x03ndn\x08\x04test"
    interest = pyndnlite.byteArray(len(name))
    for i, v in enumerate(name):
        interest[i] = ord(v)
    on_interest_func = OnInterest()
    val = pyndnlite.ndn_forwarder_register_prefix_wrapper(interest, len(name), on_interest_func)
    if val != pyndnlite.NDN_SUCCESS:
        print("ERROR!")
        running = False

    while running:
        pyndnlite.ndn_forwarder_process()
        time.sleep(0.010)


if __name__ == "__main__":
    main()
