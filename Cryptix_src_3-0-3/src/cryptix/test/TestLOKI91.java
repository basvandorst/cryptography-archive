// $Id: TestLOKI91.java,v 1.1 1997/12/05 19:15:41 raif Exp $
//
// $Log: TestLOKI91.java,v $
// Revision 1.1  1997/12/05 19:15:41  raif
// *** empty log message ***
//
// 1997.12.06: RSN
// + changed spelling to LOKI91.
//
// Revision 1.3  1997/11/22 07:05:41  raif
// *** empty log message ***
//
// Revision 1.2  1997/11/22 05:59:03  iang
// core.util ==> util.core
//
// Revision 1.1  1997/11/07 05:53:26  raif
// + adapted to latest API.
//
// 1.0.1         14 Aug 1997     R. Naffah
// + use new cryptix.util classes.
//
// 1.0.0           June 1997     R. Naffah
// + Original version.
//
/*
 * Copyright (c) 1997 Systemics Ltd
 * on behalf of the Cryptix Development Team. All rights reserved.
 */
package cryptix.test;

import cryptix.provider.key.RawSecretKey;
import cryptix.util.core.ArrayUtil;
import cryptix.util.core.Hex;

import java.security.Cipher;
import java.security.KeyException;

/**
 * Tests the output of the LOKI91 cipher algorithm implementation against
 * certified pre-computed output for a given set of reference input.
 * <p>
 * <b>Copyright</b> &copy; 1997
 * <a href="http://www.systemics.com/">Systemics Ltd</a> on behalf of the
 * <a href="http://www.systemics.com/docs/cryptix/">Cryptix Development Team</a>.
 * <br>All rights reserved.
 * <p>
 * <b>$Revision: 1.1 $</b>
 * @author  Raif S. Naffah
 */
public final class TestLOKI91
{
// Variables and constants
//............................................................................

    private static Cipher alg;

    private static final String[][] data = {
    //    key                 plain text          cipher text (ECB)
    //    ................    ..................  ..................
    //    weak keys
        {"0000000000000000", "0000000000000000", "BD84A2085EF609C7"},
        {"0000000000000000", "BD84A2085EF609C7", "0000000000000000"},
        {"FFFFFFFFFFFFFFFF", "0000000000000000", "5C77E002D1991C4D"},
        {"FFFFFFFFFFFFFFFF", "5C77E002D1991C4D", "0000000000000000"},
        {"55555555AAAAAAAA", "0000000000000000", "71FD6DC44BF4E881"},
        {"55555555AAAAAAAA", "71FD6DC44BF4E881", "0000000000000000"},
        {"AAAAAAAA55555555", "0000000000000000", "65B38FDC551B2576"},
        {"AAAAAAAA55555555", "65B38FDC551B2576", "0000000000000000"},

    //    semi-weak keys
        {"0000000055555555", "0000000000000000", "85D84CAD08342044"},
        {"0000000055555555", "8621CCB894705F8F", "0000000000000000"},
        {"AAAAAAAA00000000", "0000000000000000", "8621CCB894705F8F"},
        {"AAAAAAAA00000000", "85D84CAD08342044", "0000000000000000"},

        {"00000000AAAAAAAA", "0000000000000000", "971FE23B8904399B"},
        {"00000000AAAAAAAA", "E10C4EFC9D4C9B52", "0000000000000000"},
        {"5555555500000000", "0000000000000000", "E10C4EFC9D4C9B52"},
        {"5555555500000000", "971FE23B8904399B", "0000000000000000"},

        {"00000000FFFFFFFF", "0000000000000000", "CA30A526E3BF6FE9"},
        {"00000000FFFFFFFF", "9CC24ACE4D477F39", "0000000000000000"},
        {"FFFFFFFF00000000", "0000000000000000", "9CC24ACE4D477F39"},
        {"FFFFFFFF00000000", "CA30A526E3BF6FE9", "0000000000000000"},

        {"5555555555555555", "0000000000000000", "BC85DE5E486D7DDB"},
        {"5555555555555555", "504D4C4FEA439B63", "0000000000000000"},
        {"AAAAAAAAAAAAAAAA", "0000000000000000", "504D4C4FEA439B63"},
        {"AAAAAAAAAAAAAAAA", "BC85DE5E486D7DDB", "0000000000000000"},

        {"55555555FFFFFFFF", "0000000000000000", "0BEF28021CD23FCB"},
        {"55555555FFFFFFFF", "BAA5521BD8FF9F75", "0000000000000000"},
        {"FFFFFFFFAAAAAAAA", "0000000000000000", "BAA5521BD8FF9F75"},
        {"FFFFFFFFAAAAAAAA", "0BEF28021CD23FCB", "0000000000000000"},

        {"AAAAAAAAFFFFFFFF", "0000000000000000", "A78C6F84509C4D9A"},
        {"AAAAAAAAFFFFFFFF", "05DAC4A04D83C0AB", "0000000000000000"},
        {"FFFFFFFF55555555", "0000000000000000", "05DAC4A04D83C0AB"},
        {"FFFFFFFF55555555", "A78C6F84509C4D9A", "0000000000000000"},

    // inversion of all the key and plain-text bits results in the
    // inversion of all ciphertext bits.

        {"0123456789ABCDEF", "0000000000000000", "D26DE3321AAA29F6"},
        {"0123456789ABCDEF", "FCF597B7DA5A3A3D", "0000000000000000"},
        {"FEDCBA9876543210", "FFFFFFFFFFFFFFFF", "2D921CCDE555D609"},

    // other assorted values

        {"0000000000000000", "0000000000000000", "BD84A2085EF609C7"},
        {"0000000000000000", "BD84A2085EF609C7", "0000000000000000"},
        {"0000000000000000", "355550B2150E2451", "3644DD20D5BA5D10"},
        {"0000000000000000", "3644DD20D5BA5D10", "355550B2150E2451"},
        {"0000000000000000", "35A7BAE825C0D73B", "826FF28DB7EDF0A3"},
        {"0000000000000000", "826FF28DB7EDF0A3", "35A7BAE825C0D73B"},
        {"0000000000000000", "8CA64DE9C1B123A7", "FB3038A35A93646B"},
        {"0000000000000000", "FB3038A35A93646B", "8CA64DE9C1B123A7"},
        {"0000000000000000", "8E2A251B94704C69", "4829A2FAF34973E8"},
        {"0000000000000000", "4829A2FAF34973E8", "8E2A251B94704C69"},
        {"0000000000000000", "FFFFFFFFFFFFFFFF", "A3881FFD2E66E3B2"},
        {"0000000000000000", "A3881FFD2E66E3B2", "FFFFFFFFFFFFFFFF"},
        {"0101010101010101", "0123456789ABCDEF", "7935B683EADA00F4"},
        {"0101010101010101", "EC5960C9D3BF1E1E", "0123456789ABCDEF"},
        {"0101010101010101", "617B3A0CE8F07100", "A6090C5F651D84FF"},
        {"0101010101010101", "8EEB7798A291B0B3", "617B3A0CE8F07100"},
        {"0101010101010101", "9B38F6CE85AAB9C3", "2165FC9B5A4430D4"},
        {"0101010101010101", "2143BADADBD4F539", "9B38F6CE85AAB9C3"},
        {"0113B970FD34F2CE", "059B5E0851CF143A", "F1F6DAA4368D32C0"},
        {"0113B970FD34F2CE", "313B8C589BCF2955", "059B5E0851CF143A"},
        {"0113B970FD34F2CE", "7514CDB961B6760D", "A2F3F0E425F6CD30"},
        {"0113B970FD34F2CE", "31B8050DEFD959CE", "7514CDB961B6760D"},
        {"0113B970FD34F2CE", "86A560F10EC6D85B", "A27506525CF66E5C"},
        {"0113B970FD34F2CE", "6566F294DAF82C26", "86A560F10EC6D85B"},
        {"0123456789ABCDEF", "0000000000000000", "D26DE3321AAA29F6"},
        {"0123456789ABCDEF", "FCF597B7DA5A3A3D", "0000000000000000"},
        {"0123456789ABCDEF", "1111111111111111", "E1DD3B59134F32AB"},
        {"0123456789ABCDEF", "C7C0EA4A6A9D3360", "1111111111111111"},
        {"0123456789ABCDEF", "17668DFC7292532D", "7AAB3B9A42FECEB2"},
        {"0123456789ABCDEF", "0A9462FA8FEE7F4A", "17668DFC7292532D"},
        {"0123456789ABCDEF", "23C086665917B8E1", "9FBC1A193DCEE02D"},
        {"0123456789ABCDEF", "1CB728F0C1B26F75", "23C086665917B8E1"},
        {"0123456789ABCDEF", "D5D44FF720683D0D", "621F20A1884677F3"},
        {"0123456789ABCDEF", "C8C9A1ACE8CF89DC", "D5D44FF720683D0D"},
        {"0123456789ABCDEF", "FCE30226576320BD", "5DA683DA7E9FD159"},
        {"0123456789ABCDEF", "5EDF1C8466CE6CCA", "FCE30226576320BD"},
        {"0131D9619DC1376E", "5CD54CA83DEF57DA", "018781DEB9EA97CC"},
        {"0131D9619DC1376E", "BD1084A4B6D1AB38", "5CD54CA83DEF57DA"},
        {"0131D9619DC1376E", "65E160AED7B773A9", "57E3C651E72C6D99"},
        {"0131D9619DC1376E", "AEAB88168E9C0E3E", "65E160AED7B773A9"},
        {"0131D9619DC1376E", "7A389D10354BD271", "5520951007AC3123"},
        {"0131D9619DC1376E", "78EADDBFCB1CA8F2", "7A389D10354BD271"},
        {"0170F175468FB5E6", "0756D8E0774761D2", "AB786040F925D13E"},
        {"0170F175468FB5E6", "B7C1284CA7043DCB", "0756D8E0774761D2"},
        {"0170F175468FB5E6", "0CD3DA020021DC09", "77CE52C63517B689"},
        {"0170F175468FB5E6", "802191063DD8FF06", "0CD3DA020021DC09"},
        {"0170F175468FB5E6", "914C1806FCCBCE33", "DD546F5263780633"},
        {"0170F175468FB5E6", "5356FADAB9832B2C", "914C1806FCCBCE33"},
        {"018310DC409B26D6", "1D9D5C5018F728C2", "80D1106E854F7296"},
        {"018310DC409B26D6", "0EB06D867B7034B3", "1D9D5C5018F728C2"},
        {"018310DC409B26D6", "5A0BF934FD6009F8", "F67C664125809846"},
        {"018310DC409B26D6", "9C1EDF390E47F09A", "5A0BF934FD6009F8"},
        {"018310DC409B26D6", "5F4C038ED12B2E41", "20A61C3BC186B0B2"},
        {"018310DC409B26D6", "E93AE52452B20098", "5F4C038ED12B2E41"},
        {"025816164629B007", "480D39006EE762F2", "8197CC96E066F971"},
        {"025816164629B007", "8E67EFE514FA5985", "480D39006EE762F2"},
        {"025816164629B007", "A1F9915541020B56", "0E1B15AA45DAAD23"},
        {"025816164629B007", "7B1BD88C59347EE4", "A1F9915541020B56"},
        {"025816164629B007", "EC92E65DA168B46F", "FD5D36553CC7CDAD"},
        {"025816164629B007", "10F09CE8EC84F207", "EC92E65DA168B46F"},
        {"04689104C2FD3B2F", "26955F6835AF609A", "082133FD6BFA1E88"},
        {"04689104C2FD3B2F", "36153DA8E8E4A67A", "26955F6835AF609A"},
        {"04689104C2FD3B2F", "5265227FE08A28EC", "1B03084901B20C14"},
        {"04689104C2FD3B2F", "F2E7E1A4678D10A7", "5265227FE08A28EC"},
        {"04689104C2FD3B2F", "5C513C9C4886C088", "DB8E496A8AA4EAE8"},
        {"04689104C2FD3B2F", "49A2827DB6293C72", "5C513C9C4886C088"},
        {"04B915BA43FEB5B6", "42FD443059577FA2", "B794C8FEC41D93FD"},
        {"04B915BA43FEB5B6", "688762C5782C8D46", "42FD443059577FA2"},
        {"04B915BA43FEB5B6", "A483EA7CCF2E0E5A", "1319804D5E6477FB"},
        {"04B915BA43FEB5B6", "A4407673E0289704", "A483EA7CCF2E0E5A"},
        {"04B915BA43FEB5B6", "AF37FB421F8C4095", "5E1FBECA6D3DD240"},
        {"04B915BA43FEB5B6", "76F8A9D3D646B45A", "AF37FB421F8C4095"},
        {"07A1133E4A0B2686", "0248D43806F67172", "BDC50F07265F98BB"},
        {"07A1133E4A0B2686", "5352EA74D4F98D17", "0248D43806F67172"},
        {"07A1133E4A0B2686", "624F2E2DFA008142", "43966FBA71697D9E"},
        {"07A1133E4A0B2686", "94E8F64562868D44", "624F2E2DFA008142"},
        {"07A1133E4A0B2686", "868EBB51CAB4599A", "2BC4D7AC9F09E2CF"},
        {"07A1133E4A0B2686", "38DF7D98C5841CB1", "868EBB51CAB4599A"},
        {"07A7137045DA2A16", "28E686668C3BD6D9", "5DDB3C7FC1C8CE22"},
        {"07A7137045DA2A16", "A725D943D567693C", "28E686668C3BD6D9"},
        {"07A7137045DA2A16", "3BDD119049372802", "3F0DB5539D36D6FF"},
        {"07A7137045DA2A16", "E617587EF5EC4992", "3BDD119049372802"},
        {"07A7137045DA2A16", "DFD64A815CAF1A0F", "A4AF3B0773DAD9BE"},
        {"07A7137045DA2A16", "3000A06ACCA1E95C", "DFD64A815CAF1A0F"},
        {"1111111111111111", "0123456789ABCDEF", "75F0C8BC2BAFD309"},
        {"1111111111111111", "46B19D48D8569E55", "0123456789ABCDEF"},
        {"1111111111111111", "1111111111111111", "CC8A79BCCAE0BB7E"},
        {"1111111111111111", "D96E387165E701BC", "1111111111111111"},
        {"1111111111111111", "24900548C21A3567", "93F5DBD7F154B9BA"},
        {"1111111111111111", "66B41B7D36C436DE", "24900548C21A3567"},
        {"1111111111111111", "8A5AE1F81AB8F2DD", "20F677BEBABE1775"},
        {"1111111111111111", "D96FAF34F9F90454", "8A5AE1F81AB8F2DD"},
        {"1111111111111111", "A273D7CB7D390531", "70AB8B1BC85A3535"},
        {"1111111111111111", "44260EE8C8EEB426", "A273D7CB7D390531"},
        {"1111111111111111", "F40379AB9E0EC533", "06F8080AE1393C2F"},
        {"1111111111111111", "9AE34DE86300215F", "F40379AB9E0EC533"},
        {"1C587F1C13924FEF", "305532286D6F295A", "2FC181B0CB6B30B7"},
        {"1C587F1C13924FEF", "E9EEF1EB68E8C676", "305532286D6F295A"},
        {"1C587F1C13924FEF", "63FAC0D034D9F793", "E3F0D7AFE60812B2"},
        {"1C587F1C13924FEF", "7409D13AB01F25F6", "63FAC0D034D9F793"},
        {"1C587F1C13924FEF", "F63DE067F58C38ED", "3F4913280D2793FC"},
        {"1C587F1C13924FEF", "0109ECCD8A9C40DD", "F63DE067F58C38ED"},
        {"1F08260D1AC2465E", "2C0A241EB9F05999", "E86F92AC2EC186F3"},
        {"1F08260D1AC2465E", "2DBAFAAF1EB7B072", "2C0A241EB9F05999"},
        {"1F08260D1AC2465E", "6B056E18759F5CCA", "02DD8970249255F1"},
        {"1F08260D1AC2465E", "377F235592B45CA9", "6B056E18759F5CCA"},
        {"1F08260D1AC2465E", "EF1BF03E5DFA575A", "98E1E5771C4A9455"},
        {"1F08260D1AC2465E", "F489970BB6E8A94E", "EF1BF03E5DFA575A"},
        {"1F1F1F1F0E0E0E0E", "0123456789ABCDEF", "1998D96B0BB0ADD6"},
        {"1F1F1F1F0E0E0E0E", "7454D8D7C56BE016", "0123456789ABCDEF"},
        {"1F1F1F1F0E0E0E0E", "322F206B2D39D65D", "0B636EB981E987C0"},
        {"1F1F1F1F0E0E0E0E", "0B926C6049361BFF", "322F206B2D39D65D"},
        {"1F1F1F1F0E0E0E0E", "DB958605F8C8C606", "5A153AEA62BDD0CB"},
        {"1F1F1F1F0E0E0E0E", "2C7BF241AA0B2C97", "DB958605F8C8C606"},
        {"3000000000000000", "1000000000000001", "12F2D0471DE69A16"},
        {"3000000000000000", "B0EF21AD8B82DD0A", "1000000000000001"},
        {"3000000000000000", "958E6E627A05557B", "4BE1B16EF2C9524A"},
        {"3000000000000000", "437B3AC6A2A027C8", "958E6E627A05557B"},
        {"3000000000000000", "D3C4539579B96231", "BBFCAF9CC8CD9E46"},
        {"3000000000000000", "7B8B6B2E19E13C69", "D3C4539579B96231"},
        {"37D06BB516CB7546", "0A2AEEAE3FF4AB77", "32A65F25341D17F0"},
        {"37D06BB516CB7546", "90D6DCEE0F2F8D62", "0A2AEEAE3FF4AB77"},
        {"37D06BB516CB7546", "164D5E404F275232", "782A69EC6256C642"},
        {"37D06BB516CB7546", "1DABF24C62F0FD89", "164D5E404F275232"},
        {"37D06BB516CB7546", "19ACAE3136C0BC7C", "9CD87A6A8EA4C562"},
        {"37D06BB516CB7546", "F8BF4D34438681A2", "19ACAE3136C0BC7C"},
        {"3849674C2602319E", "126898D55E911500", "C86CAEC1E3B7B17E"},
        {"3849674C2602319E", "D8D28161851ED6D6", "126898D55E911500"},
        {"3849674C2602319E", "51454B582DDF440A", "8904C12A608BAC74"},
        {"3849674C2602319E", "562DE7A88B1F4877", "51454B582DDF440A"},
        {"3849674C2602319E", "7178876E01F19B2A", "BA940CD1789E92FD"},
        {"3849674C2602319E", "295ECE9867B10E95", "7178876E01F19B2A"},
        {"43297FAD38E373FE", "4C974F1CAA59F5D4", "71A64728851D6E14"},
        {"43297FAD38E373FE", "F386803C5A434216", "4C974F1CAA59F5D4"},
        {"43297FAD38E373FE", "762514B829BF486A", "6684FB847E1283C0"},
        {"43297FAD38E373FE", "BF8E6FBF6C48EC44", "762514B829BF486A"},
        {"43297FAD38E373FE", "EA676B2CB7DB2B7A", "5A2D250D39EB8FC1"},
        {"43297FAD38E373FE", "6D42D3B3B5A9ABCC", "EA676B2CB7DB2B7A"},
        {"49793EBC79B3258F", "437540C8698F3CFA", "0B46E6CD65118775"},
        {"49793EBC79B3258F", "B9D425D544C1CE71", "437540C8698F3CFA"},
        {"49793EBC79B3258F", "6FBF1CAFCFFD0556", "50BFA6E233CEF0AF"},
        {"49793EBC79B3258F", "000C45276715001D", "6FBF1CAFCFFD0556"},
        {"49793EBC79B3258F", "A0CB2871752053F0", "AF27090A887C7E3E"},
        {"49793EBC79B3258F", "51FC08AFEC3C8F1A", "A0CB2871752053F0"},
        {"49E95D6D4CA229BF", "00B0024EAAC70AE3", "6F2421DC83DF9BA2"},
        {"49E95D6D4CA229BF", "82537CE99E3476DE", "00B0024EAAC70AE3"},
        {"49E95D6D4CA229BF", "02FE55778117F12A", "BD6BB416626BE321"},
        {"49E95D6D4CA229BF", "417B8CDD1C64B3E7", "02FE55778117F12A"},
        {"49E95D6D4CA229BF", "5A6B612CC26CCE4A", "73CB63595026B590"},
        {"49E95D6D4CA229BF", "A90F566D8F4E3E41", "5A6B612CC26CCE4A"},
        {"4FB05E1515AB73A7", "072D43A077075292", "AC6174B535BCAD69"},
        {"4FB05E1515AB73A7", "2D139D3801A19346", "072D43A077075292"},
        {"4FB05E1515AB73A7", "2F22E49BAB7CA1AC", "86019DD993C46CBE"},
        {"4FB05E1515AB73A7", "44FEF33E6971A69B", "2F22E49BAB7CA1AC"},
        {"4FB05E1515AB73A7", "AD87789BC00718C2", "733B12F600953A98"},
        {"4FB05E1515AB73A7", "8341990D8BA44544", "AD87789BC00718C2"},
        {"584023641ABA6176", "004BD6EF09176062", "6CECA0E2D5351CD1"},
        {"584023641ABA6176", "9C6AEDFFD92B1CCA", "004BD6EF09176062"},
        {"584023641ABA6176", "88BF0DB6D70DEE56", "B3A4719ECA678ECC"},
        {"584023641ABA6176", "BB39BB8BDF9DB594", "88BF0DB6D70DEE56"},
        {"584023641ABA6176", "C7D2845EA6D01C70", "2932E00DA18DB422"},
        {"584023641ABA6176", "2217A63228E547C1", "C7D2845EA6D01C70"},
        {"5B5A57676A56676E", "974AFFBF86022D1F", "6B6413098AAA7F2B"},
        {"5B5A57676A56676E", "DAABFAB98DEA5E78", "974AFFBF86022D1F"},
        {"7CA110454A1A6E57", "01A1D6D039776742", "0200C0B643578183"},
        {"7CA110454A1A6E57", "1405433E91265B1B", "01A1D6D039776742"},
        {"7CA110454A1A6E57", "690F5B0D9A26939B", "D6173B3A2F754508"},
        {"7CA110454A1A6E57", "9F7C523C7BFE7586", "690F5B0D9A26939B"},
        {"7CA110454A1A6E57", "ECD1C2F929F33CED", "A5D38784EF7973EE"},
        {"7CA110454A1A6E57", "C6642999C37B99B3", "ECD1C2F929F33CED"},
        {"E0FEE0FEF1FEF1FE", "0123456789ABCDEF", "E09CA157C6DFE236"},
        {"E0FEE0FEF1FEF1FE", "20BC4D004E45C048", "0123456789ABCDEF"},
        {"E0FEE0FEF1FEF1FE", "B7687FACF9B1A656", "EB2C523BE4DEDD8A"},
        {"E0FEE0FEF1FEF1FE", "C21C42EF608F5AE6", "B7687FACF9B1A656"},
        {"E0FEE0FEF1FEF1FE", "EDBFD1C66C29CCC7", "D7A84A5234F98995"},
        {"E0FEE0FEF1FEF1FE", "CB3F0E23D4AD3760", "EDBFD1C66C29CCC7"},
        {"FEDCBA9876543210", "0123456789ABCDEF", "480F0EAC8D4C6CBC"},
        {"FEDCBA9876543210", "25D00012F501ADAF", "0123456789ABCDEF"},
        {"FEDCBA9876543210", "2A2BB008DF97C2F2", "9DE0DF5E77B9880C"},
        {"FEDCBA9876543210", "37365E5317307623", "2A2BB008DF97C2F2"},
        {"FEDCBA9876543210", "7F46AA73F7FCDE02", "F6B3C639D4588687"},
        {"FEDCBA9876543210", "286334C90FC54D99", "7F46AA73F7FCDE02"},
        {"FEDCBA9876543210", "C44C1B3668A0F2CF", "F1FBE0DB72AF2072"},
        {"FEDCBA9876543210", "4DA710EA766DDDB4", "C44C1B3668A0F2CF"},
        {"FEDCBA9876543210", "ED39D950FA74BCC4", "1529A5409E492728"},
        {"FEDCBA9876543210", "BA2FCBDF566D6B40", "ED39D950FA74BCC4"},
        {"FEDCBA9876543210", "FFFFFFFFFFFFFFFF", "2D921CCDE555D609"},
        {"FEDCBA9876543210", "030A684825A5C5C2", "FFFFFFFFFFFFFFFF"},
        {"FFFFFFFFFFFFFFFF", "0000000000000000", "5C77E002D1991C4D"},
        {"FFFFFFFFFFFFFFFF", "5C77E002D1991C4D", "0000000000000000"},
        {"FFFFFFFFFFFFFFFF", "16B15028F06A5AB8", "C594D15528F44560"},
        {"FFFFFFFFFFFFFFFF", "C594D15528F44560", "16B15028F06A5AB8"},
        {"FFFFFFFFFFFFFFFF", "3C7188775253884D", "AC490AEF39A87B61"},
        {"FFFFFFFFFFFFFFFF", "AC490AEF39A87B61", "3C7188775253884D"},
        {"FFFFFFFFFFFFFFFF", "7359B2163E4EDC58", "04CFC75CA56C9B94"},
        {"FFFFFFFFFFFFFFFF", "04CFC75CA56C9B94", "7359B2163E4EDC58"},
        {"FFFFFFFFFFFFFFFF", "CAAAAF4DEAF1DBAE", "C9BB22DF2A45A2EF"},
        {"FFFFFFFFFFFFFFFF", "C9BB22DF2A45A2EF", "CAAAAF4DEAF1DBAE"},
        {"FFFFFFFFFFFFFFFF", "FFFFFFFFFFFFFFFF", "427B5DF7A109F638"},
        {"FFFFFFFFFFFFFFFF", "427B5DF7A109F638", "FFFFFFFFFFFFFFFF"}
    };

// Constructor
//...........................................................................
    
    private TestLOKI91 () {
        System.out.println("*** LOKI-91:\n");
        try {
            test1();
            test2();
        }
        catch (Throwable t) { t.printStackTrace(); }
    }


// main/test methods
//...........................................................................
    
    public static final void main (String[] args) {
        try {
            alg = Cipher.getInstance("LOKI-91", "Cryptix");
            TestLOKI91 x = new TestLOKI91();
        }
        catch (Throwable t) { t.printStackTrace(); }
    }
    
    private void test1 ()
    throws Exception {
        byte[]
            key = Hex.fromString("3849674c2602319e"),
            input = Hex.fromString("126898d55e911500"),
            output = Hex.fromString("c86caec1e3b7b17e");
        RawSecretKey aKey = new RawSecretKey("Loki91", key);

        System.out.println("\nTest vector (single):\nEncrypting:");

        alg.initEncrypt(aKey);
        compareIt(alg.crypt(input), output);

        System.out.println("\nDecrypting:");
        alg.initDecrypt(aKey);
        compareIt(alg.crypt(output), input);
    }

    private void test2 ()
    throws Exception {
        byte[] ect, dct;
        String a, b;
        RawSecretKey key;

        System.out.println("\nTest vectors (triplets):\n");
        System.out.println("Key              Plain-text       Cipher-text      Enc.   Dec.");
        System.out.println("................ ................ ................ ...... ......");
        for (int i = 0; i < data.length; i++) {
            try {
                key = new RawSecretKey("LOKI91", Hex.fromString(data[i][0]));
                System.out.print(data[i][0] + " ");
        
                alg.initEncrypt(key);
                System.out.print(data[i][1] + " ");
                System.out.print(data[i][2] + " ");
                ect = alg.crypt(Hex.fromString(data[i][1]));
                a = Hex.toString(ect);
                System.out.print(a.equals(data[i][2]) ? "GOOD   " : "FAILED ");

                alg.initDecrypt(key);
                dct = alg.crypt(Hex.fromString(data[i][2]));
                b = Hex.toString(dct);
                System.out.println(b.equals(data[i][1]) ? "GOOD" : "FAILED");
            }
            catch (KeyException e) { System.out.println("*** " + e); }
        }
    }

    private void compareIt (byte[] o1, byte[] o2) {
        if (o1.length < 33) {
            System.out.println("  computed: " + Hex.toString(o1));
            System.out.println(" certified: " + Hex.toString(o2));
        } else {
            System.out.println("  computed: " + Hex.dumpString(o1));
            System.out.println(" certified: " + Hex.dumpString(o2));
        }
        System.out.println(ArrayUtil.areEqual(o1, o2) ? " *** GOOD" : " *** FAILED");
    }
}
